/**
 * @file Tests for the `Client` class.
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

#include "../client.h"
#include "test_messages.pb.h"
#include "thread_pool/thread_pool.h"
#include "wire_protocol/wire_protocol.h"

namespace message_passing::tests {
namespace {

using test_messages::TestMessage;
using test_messages::TestResponse;
using thread_pool::ThreadPool;
using wire_protocol::MessageParser;

/// Parameter string to use for test messages.
const char *kTestParameterString = "a parameter string value";
/// Endpoint to use for testing.
const Endpoint kTestEndpoint = {"127.0.0.1", 1234};

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
 * @brief Creates a response message to use for testing.
 * @return The message that it created.
 */
TestResponse MakeTestResponse() {
  TestResponse test_message;
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
 * @param send_response If true, will send a response message.
 * @param received_message[out] The received message.
 */
void RunServer(int server_socket, bool send_response,
               TestMessage *received_message) {
  // Accept a client connection.
  int client_fd = accept(server_socket, nullptr, nullptr);

  // Parser to use for received messages.
  MessageParser<TestMessage> parser;
  // Buffer to use for received message data.
  constexpr uint32_t kReceiveBufferSize = 1024;
  std::vector<uint8_t> receive_buffer;

  while (!parser.HasCompleteMessage()) {
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
  }

  // Save the message we got.
  parser.GetMessage(received_message);

  if (send_response) {
    // Send the response message.
    std::vector<uint8_t> serialized;
    wire_protocol::Serialize(MakeTestResponse(), &serialized);
    if (send(client_fd, serialized.data(), serialized.size(), 0) < 0) {
      LOG_S(ERROR) << "Failed to send response: " << std::strerror(errno);
    }
  }

  // For testing, we force it to immediately send a reset to the sender.
  const struct linger kOptions = {1, 0};
  setsockopt(client_fd, SOL_SOCKET, SO_LINGER, &kOptions, sizeof(kOptions));
  close(client_fd);
  close(server_socket);
}

/**
 * @brief Main function for a server that sends a set of messages and then
 * exits.
 * @param server_socket The server socket to listen on.
 * @param messages The messages to send.
 */
void SendMessages(int server_socket,
                  const std::vector<TestResponse> &messages) {
  // Accept a client connection.
  int client_fd = accept(server_socket, nullptr, nullptr);

  std::vector<uint8_t> serialized;
  for (const auto &message : messages) {
    wire_protocol::Serialize(message, &serialized);
    if (send(client_fd, serialized.data(), serialized.size(), 0) < 0) {
      LOG_S(ERROR) << "Failed to send message: " << std::strerror(errno);
    }
  }

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
   * @param send_response If true, will send a response after receiving the
   *    message.
   */
  explicit SingleShotServer(uint16_t port, bool send_response = false)
      : port_(port), send_response_(send_response) {}
  ~SingleShotServer() {
    if (server_thread_.joinable()) {
      server_thread_.join();
    }
  }

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

    server_thread_ =
        std::thread(RunServer, kServerFd, send_response_, &received_message_);

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
  /// Whether to send a response.
  bool send_response_;

  /// Thread that runs the server.
  std::thread server_thread_{};
  /// Queue to store received messages.
  TestMessage received_message_;
};

/**
 * @brief This class listens on a port, accepts a single connection,
 *  and sends a bunch of messages to it before disconnecting.
 */
class MessageSendingServer {
 public:
  /**
   * @param port The port to listen on.
   * @param messages The messages to send.
   */
  MessageSendingServer(uint16_t port, std::vector<TestResponse> messages)
      : port_(port), messages_(std::move(messages)) {}
  ~MessageSendingServer() {
    if (server_thread_.joinable()) {
      server_thread_.join();
    }
  }

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

    server_thread_ = std::thread(SendMessages, kServerFd, messages_);

    return true;
  }

  /**
   * @brief Waits for the server to complete.
   */
  void Join() { server_thread_.join(); }

 private:
  /// The port to listen on.
  uint16_t port_;
  /// The messages to send.
  std::vector<TestResponse> messages_;
  /// Thread that runs the server.
  std::thread server_thread_{};
};

/**
 * @brief Encapsulates standard configuration for tests.
 */
struct ConfigForTests {
  /// The thread pool to use internally.
  std::shared_ptr<ThreadPool> thread_pool;
  /// The actual Client under test.
  std::unique_ptr<Client> client;
};

/**
 * @brief Creates standard configuration for tests.
 * @return The configuration that it created.
 */
ConfigForTests MakeConfig() {
  auto thread_pool = std::make_shared<ThreadPool>();
  auto message_sender = std::make_unique<Client>(thread_pool, kTestEndpoint);

  return {thread_pool, std::move(message_sender)};
}

/**
 * @test Tests that we can send a single message asynchronously.
 */
TEST(Client, SendSingleMessageAsync) {
  // Arrange.
  auto config = MakeConfig();

  // Listen for the message.
  SingleShotServer server(kTestEndpoint.port);
  ASSERT_TRUE(server.Begin());

  // Act.
  // Send the message.
  config.client->SendAsync(MakeTestMessage());

  // Assert.
  // Wait for the message to arrive.
  const auto &kGotMessage = server.GetMessage();
  // It should have gotten the message we expect.
  EXPECT_EQ(kTestParameterString, kGotMessage.parameter());
}

/**
 * @test Tests that we can send a single message synchronously.
 */
TEST(Client, SendSingleMessageSync) {
  // Arrange.
  auto config = MakeConfig();

  // Listen for the message.
  SingleShotServer server(kTestEndpoint.port);
  ASSERT_TRUE(server.Begin());

  // Act.
  // Send the message.
  const int kSendResult = config.client->Send(MakeTestMessage());

  // Assert.
  // The send should have succeeded.
  EXPECT_GT(kSendResult, 0);
  // Wait for the message to arrive.
  const auto &kGotMessage = server.GetMessage();
  // It should have gotten the message we expect.
  EXPECT_EQ(kTestParameterString, kGotMessage.parameter());
}

/**
 * @test Tests that it handles a connection failure on send.
 */
TEST(Client, SendConnectionFailure) {
  // Arrange.
  auto config = MakeConfig();

  // Act.
  // Try to send a message.
  const int kSendResult = config.client->Send(MakeTestMessage());

  // Assert.
  // The send should have failed.
  EXPECT_LT(kSendResult, 0);
}

/**
 * @test Tests that it handles a connection failure on receive.
 */
TEST(Client, ReceiveConnectionFailure) {
  // Arrange.
  auto config = MakeConfig();

  // Act.
  // Try to receive a message.
  TestResponse response;
  const bool kReceiveResult =
      config.client->Receive(&response, nullptr);

  // Assert.
  // The receive should have failed.
  EXPECT_FALSE(kReceiveResult);
}

/**
 * @test Tests that we can successfully send a message and receive a response.
 */
TEST(Client, RequestResponse) {
  // Arrange.
  auto config = MakeConfig();

  // Listen for the message.
  SingleShotServer server(kTestEndpoint.port, true);
  ASSERT_TRUE(server.Begin());

  // Act.
  TestResponse response;
  const bool kResult =
      config.client->SendRequest(MakeTestMessage(), &response);

  // Assert.
  // It should have succeeded.
  ASSERT_TRUE(kResult);

  // It should have received the expected request.
  const auto &kReceivedMessage = server.GetMessage();
  EXPECT_EQ(kTestParameterString, kReceivedMessage.parameter());

  // It should have received the expected response.
  EXPECT_EQ(kTestParameterString, response.parameter());
}

/**
 * @test Tests that receiving with a timeout works.
 */
TEST(Client, ReceiveTimeout) {
  auto config = MakeConfig();

  // Create a server that will wait for us to send it a message. It
  // will be blocked until we do this.
  SingleShotServer server(kTestEndpoint.port);
  ASSERT_TRUE(server.Begin());

  // Act.
  // Try receiving with a timeout.
  TestResponse response;
  const bool kResult = config.client->Receive(
      std::chrono::milliseconds(100), &response, nullptr);

  // Clean up the server.
  // Send a message to it exits.
  config.client->SendAsync(MakeTestMessage());
  server.GetMessage();

  // Assert.
  // It should have timed out.
  EXPECT_FALSE(kResult);
}

/**
 * @test Tests that it handles it correctly when we receive two messages
 *  back-to-back.
 */
TEST(Client, ReceiveBackToBack) {
  // Arrange.
  auto config = MakeConfig();

  // Create a server to fire off some messages.
  auto message1 = MakeTestResponse();
  auto message2 = MakeTestResponse();
  MessageSendingServer server(kTestEndpoint.port, {message1, message2});
  ASSERT_TRUE(server.Begin());

  // Act.
  // Do a send, which will force a connection and cause the messages to be
  // received back-to-back.
  config.client->SendAsync(MakeTestMessage());

  // Wait for the messages to be sent.
  server.Join();

  // Now receive the two messages.
  TestResponse got_message_1, got_message_2;
  Endpoint source1, source2;
  ASSERT_TRUE(config.client->Receive(&got_message_1, &source1));
  ASSERT_TRUE(config.client->Receive(&got_message_2, &source2));

  // Assert.
  // It should have read the correct messages.
  EXPECT_EQ(message1.parameter(), got_message_1.parameter());
  EXPECT_EQ(message2.parameter(), got_message_2.parameter());

  // The sources should have been set correctly.
  EXPECT_EQ(kTestEndpoint, source1);
  EXPECT_EQ(kTestEndpoint, source2);
}

}  // namespace

}  // namespace message_passing::tests