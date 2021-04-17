/**
 * @file Tests for the `MessageSender` class.
 */

#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <loguru.hpp>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "../message_sender.h"
#include "test_messages.pb.h"
#include "thread_pool/thread_pool.h"
#include "wire_protocol/wire_protocol.h"

namespace message_passing::tests {
namespace {

using test_messages::TestMessage;
using thread_pool::ThreadPool;
using wire_protocol::MessageParser;

/// Parameter string to use for test messages.
const char *kTestParameterString = "a parameter string value";

/**
 * @brief Creates a message to use for testing.
 * @return The message that it created.
 */
TestMessage MakeTestMessage() {
  TestMessage test_message;
  test_message.set_parameter(kTestParameterString);

  return test_message;
}

/**
 * @brief Creates the address structure to use for socket creation.
 * @param port The port we want to use.
 * @return The address structure it created.
 */
struct sockaddr_in MakeAddress(uint16_t port) {
  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  return address;
}

/**
 * @brief Sets up a server socket for listening.
 * @param address The address structure to use.
 * @return The server socket it created, or -1 if it failed.
 */
int SetUpListenerSocket(const struct sockaddr_in &address) {
  // Open a TCP socket.
  const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    LOG_S(ERROR) << "Failed to create server socket";
    return -1;
  }

  // Allow the server to re-bind to this port if it was restarted quickly.
  const int option = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option,
                 sizeof(option))) {
    LOG_S(ERROR) << "Failed to set socket options";
    // This is not a fatal error.
  }

  // Bind to the port.
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    LOG_S(ERROR) << "bind() failed on server socket";
    return -1;
  }
  if (listen(server_fd, 1) < 0) {
    LOG_S(ERROR) << "listen() failed on server socket";
    return -1;
  }

  return server_fd;
}

/**
 * @brief Runs the server until it receives a message or the server disconnects.
 * @param server_socket The server socket to listen on.
 * @param received_message The received message.
 */
void RunServer(int server_socket, TestMessage *received_message) {
  // Accept a client connection.
  int client_fd = accept(server_socket, nullptr, nullptr);

  // Parser to use for received messages.
  MessageParser<TestMessage> parser;
  // Buffer to use for received message data.
  constexpr uint32_t kReceiveBufferSize = 1024;
  std::vector<uint8_t> receive_buffer;

  while (true) {
    receive_buffer.resize(kReceiveBufferSize);

    // Read the next message.
    const int kBytesRead =
        recv(client_fd, receive_buffer.data(), kReceiveBufferSize, 0);

    if (kBytesRead < 0) {
      LOG_S(ERROR) << "Failed to read from client socket: "
                   << std::strerror(errno);
      break;
    } else if (kBytesRead == 0) {
      // Client disconnect. Exit.
      break;
    }

    // The parser expects the message to take up the entire buffer.
    receive_buffer.resize(kBytesRead);
    parser.AddNewData(receive_buffer);
    if (parser.HasCompleteMessage()) {
      // Save the message we got.
      parser.GetMessage(received_message);
      break;
    }
  }

  // For testing, we force it to immediately send a reset to the sender.
  const struct linger kOptions = {1, 0};
  setsockopt(client_fd, SOL_SOCKET, SO_LINGER, &kOptions, sizeof(kOptions));
  close(client_fd);
  close(server_socket);
}

/**
 * @brief This class listens on a port, accepts one connection, reads
 * a single message from the client, and then terminates the server.
 */
class SingleShotServer {
 public:
  /**
   * @param port The port to listen on.
   */
  explicit SingleShotServer(uint16_t port) : port_(port) {}

  /**
   * @brief Begins running the server in a separate thread.
   * @return True if it successfully started the server.
   */
  bool Begin() {
    // Open the server socket.
    const auto kAddress = MakeAddress(port_);
    const int kServerFd = SetUpListenerSocket(kAddress);
    if (kServerFd < 0) {
      return false;
    }

    server_thread_ = std::thread(RunServer, kServerFd, &received_message_);

    return true;
  }

  /**
   * @brief Waits for the server to terminate and gets the received messages.
   * @return The received message.
   */
  const TestMessage &GetMessage() {
    server_thread_.join();
    return received_message_;
  }

 private:
  /// The port to listen on.
  uint16_t port_;

  /// Thread that runs the server.
  std::thread server_thread_{};
  /// Queue to store received messages.
  TestMessage received_message_;
};

/**
 * @brief Encapsulates standard configuration for tests.
 */
struct ConfigForTests {
  /// The thread pool to use internally.
  std::shared_ptr<ThreadPool> thread_pool;
  /// The actual MessageSender under test.
  std::unique_ptr<MessageSender> message_sender;
};

/**
 * @brief Creates standard configuration for tests.
 * @return The configuration that it created.
 */
ConfigForTests MakeConfig() {
  auto thread_pool = std::make_shared<ThreadPool>();
  auto message_sender = std::make_unique<MessageSender>(thread_pool);

  return {thread_pool, std::move(message_sender)};
}

/**
 * @test Tests that we can send a single message asynchronously.
 */
TEST(MessageSender, SingleMessageAsync) {
  // Arrange.
  auto config = MakeConfig();

  // Listen for the message.
  SingleShotServer server(1234);
  ASSERT_TRUE(server.Begin());

  // Act.
  // Send the message.
  config.message_sender->SendAsync(MakeTestMessage(), {"127.0.0.1", 1234});

  // Assert.
  // Wait for the message to arrive.
  const auto &kGotMessage = server.GetMessage();
  // It should have gotten the message we expect.
  EXPECT_EQ(kTestParameterString, kGotMessage.parameter());
}

/**
 * @test Tests that we can send a single message synchronously.
 */
TEST(MessageSender, SingleMessageSync) {
  // Arrange.
  auto config = MakeConfig();

  // Listen for the message.
  SingleShotServer server(1234);
  ASSERT_TRUE(server.Begin());

  // Act.
  // Send the message.
  const int kSendResult =
      config.message_sender->Send(MakeTestMessage(), {"127.0.0.1", 1234});

  // Assert.
  // The send should have succeeded.
  EXPECT_GT(kSendResult, 0);
  // Wait for the message to arrive.
  const auto &kGotMessage = server.GetMessage();
  // It should have gotten the message we expect.
  EXPECT_EQ(kTestParameterString, kGotMessage.parameter());
}

/**
 * @test Tests that we can send multiple messages to the same endpoint with
 *  a disconnect in between.
 */
TEST(MessageSender, RepeatedSend) {
  // Arrange.
  auto config = MakeConfig();

  // Listen for the message.
  SingleShotServer server1(1234);
  ASSERT_TRUE(server1.Begin());

  // Act.
  // Send the message.
  const int kSendResult1 =
      config.message_sender->Send(MakeTestMessage(), {"127.0.0.1", 1234});

  // Wait for the first message to arrive, which will cause the first server
  // to exit.
  const auto &kGotMessage1 = server1.GetMessage();

  // Restart the server1 and send the message again.
  SingleShotServer server2(1234);
  ASSERT_TRUE(server2.Begin());

  // Send the message again.
  const int kSendResult2 =
      config.message_sender->Send(MakeTestMessage(), {"127.0.0.1", 1234});

  // Assert.
  // The sends should have succeeded.
  EXPECT_GT(kSendResult1, 0);
  EXPECT_GT(kSendResult2, 0);

  // Wait for the second message to arrive.
  const auto &kGotMessage2 = server2.GetMessage();
  // It should have gotten the message we expect.
  EXPECT_EQ(kTestParameterString, kGotMessage1.parameter());
  EXPECT_EQ(kTestParameterString, kGotMessage2.parameter());
}

/**
 * @test Tests that we can send messages to different endpoints concurrently.
 */
TEST(MessageSender, SendDifferendEndpoints) {
  // Arrange.
  auto config = MakeConfig();

  // Listen for the messages.
  SingleShotServer server1(1234);
  ASSERT_TRUE(server1.Begin());
  SingleShotServer server2(1235);
  ASSERT_TRUE(server2.Begin());

  // Act.
  // Send the messages.
  const int kSendResult1 =
      config.message_sender->Send(MakeTestMessage(), {"127.0.0.1", 1234});
  const int kSendResult2 =
      config.message_sender->Send(MakeTestMessage(), {"127.0.0.1", 1235});

  // Assert.
  // The sends should have succeeded.
  EXPECT_GT(kSendResult1, 0);
  EXPECT_GT(kSendResult2, 0);

  // Both messages should have been received.
  const auto &kGotMessage1 = server1.GetMessage();
  const auto &kGotMessage2 = server2.GetMessage();
  // It should have gotten the message we expect.
  EXPECT_EQ(kTestParameterString, kGotMessage1.parameter());
  EXPECT_EQ(kTestParameterString, kGotMessage2.parameter());
}

/**
 * @test Tests that it handles a connection failure.
 */
TEST(MessageSender, ConnectionFailure) {
  // Arrange.
  auto config = MakeConfig();

  // Act.
  // Try to send a message.
  const int kSendResult =
      config.message_sender->Send(MakeTestMessage(), {"127.0.0.1", 1234});

  // Assert.
  // The send should have failed.
  EXPECT_LT(kSendResult, 0);
}

}  // namespace

}  // namespace message_passing::tests