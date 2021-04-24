/**
 * @file Tests for the `Server` class.
 */

#include <gtest/gtest.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <loguru.hpp>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include "../server.h"
#include "../utils.h"
#include "test_messages.pb.h"
#include "thread_pool/thread_pool.h"
#include "wire_protocol/wire_protocol.h"

namespace message_passing::tests {
namespace {

using test_messages::TestMessage;
using test_messages::TestResponse;
using thread_pool::ThreadPool;
using wire_protocol::MessageParser;
using wire_protocol::Serialize;

/// Listening port to use for testing.
constexpr uint16_t kServerPort = 1234;
/// Parameter string to use for test messages.
const char* kTestParameterString = "a parameter string value";
/// Server endpoint to use for testing.
const Endpoint kTestEndpoint = {"127.0.0.1", kServerPort};
/// How many times to try connecting before we give up.
constexpr uint8_t kConnectionRetries = 5;
/// Socket timeout to use for testing, in seconds.
constexpr uint8_t kSocketTimeout = 30;

/// Size of chunks to receive messages in.
constexpr uint32_t kMessageChunkSize = 1024;

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
 * @brief Encapsulates standard configuration for tests.
 */
struct ConfigForTests {
  /// The thread pool to use internally.
  std::shared_ptr<ThreadPool> thread_pool;
  /// The actual server under test.
  std::unique_ptr<Server> server;
};

/**
 * @brief Creates standard configuration for tests.
 * @return The configuration that it created.
 */
ConfigForTests MakeConfig() {
  auto thread_pool = std::make_shared<ThreadPool>();
  auto server = std::make_unique<Server>(thread_pool, kServerPort);

  return {thread_pool, std::move(server)};
}

/**
 * @brief Connects to a server, with retrying.
 * @param endpoint The server address to connect to.
 * @return The FD of the client socket.
 */
int Connect(const Endpoint& endpoint) {
  const auto kAddress = MakeAddress(endpoint.port);

  // Connect to the socket with retries.
  int sock_fd = -1;
  for (uint8_t num_retries = 0; num_retries < kConnectionRetries;
       ++num_retries) {
    sock_fd = SetUpSocket(kAddress, endpoint.hostname);
    if (sock_fd < 0) {
      // Wait before trying again.
      std::this_thread::sleep_for(std::chrono::seconds(1));
    } else {
      // We connected.
      break;
    }
  }

  // Set a non-default timeout for testing.
  if (sock_fd >= 0) {
    struct timeval timeout {};
    timeout.tv_sec = kSocketTimeout;
    timeout.tv_usec = 0;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,
               sizeof(timeout));
  }

  return sock_fd;
}

/**
 * @brief Convenience function that connects to a server and sends a series of
 *  messages.
 * @param endpoint The server address to connect to.
 * @param messages The messages to send.
 * @return True if it successfully connected and sent the messages, false
 *  otherwise.
 */
bool ConnectAndSend(const Endpoint& endpoint,
                    const std::vector<TestMessage>& messages) {
  const int kSockFd = Connect(endpoint);
  if (kSockFd < 0) {
    // Failed to connect.
    return false;
  }

  // Send all the messages.
  for (const auto& kMessage : messages) {
    // Serialize first.
    std::vector<uint8_t> serialized;
    Serialize(kMessage, &serialized);

    if (send(kSockFd, serialized.data(), serialized.size(), 0) !=
        static_cast<ssize_t>(serialized.size())) {
      LOG_S(ERROR) << "send() failed during testing: " << std::strerror(errno);
      close(kSockFd);
      return false;
    }
  }

  close(kSockFd);
  return true;
}

/**
 * @brief Convenience function that connects to a server and receives a message.
 * @param endpoint The server address to connect to.
 * @param response[out] The message that it received.
 * @return True if it successfully connected and received the message, false
 *  otherwise.
 */
bool ConnectAndReceive(const Endpoint& endpoint, TestResponse* response) {
  const int kSockFd = Connect(endpoint);
  if (kSockFd < 0) {
    // Failed to connect.
    return false;
  }

  // Receive a complete message.
  MessageParser<TestResponse> parser;
  std::vector<uint8_t> received_bytes;
  while (!parser.HasCompleteMessage()) {
    received_bytes.resize(kMessageChunkSize);

    const auto kReceiveResult =
        recv(kSockFd, received_bytes.data(), kMessageChunkSize, 0);
    if (kReceiveResult < 0) {
      LOG_S(ERROR) << "recv() failed during testing: " << std::strerror(errno);
      close(kSockFd);
      return false;
    } else if (kReceiveResult == 0) {
      LOG_S(ERROR) << "Server unexpectedly disconnected during receive.";
      close(kSockFd);
      return false;
    }

    // The parser expects the message to contain no extraneous data.
    received_bytes.resize(kReceiveResult);
    parser.AddNewData(received_bytes);
  }

  close(kSockFd);
  return parser.GetMessage(response);
}

/**
 * @brief Waits for a client to connect to the server.
 * @param config The server configuration.
 * @return True if a client connected within the time limit, false otherwise.
 */
bool WaitForConnection(const ConfigForTests& config) {
  bool connected = false;
  for (uint8_t num_retries = 0; num_retries < kConnectionRetries;
       ++num_retries) {
    if (!config.server->GetConnected().empty()) {
      // It's connected. We're done.
      connected = true;
      break;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return connected;
}

}  // namespace

/**
 * @test Tests that we can receive a single message.
 */
TEST(Server, ReceiveSingleMessage) {
  // Arrange.
  auto config = MakeConfig();

  // Send a message to the server.
  const auto kTestMessage = MakeTestMessage();
  ASSERT_TRUE(ConnectAndSend(kTestEndpoint, {kTestMessage}));

  // Act.
  // Try receiving the message.
  TestMessage received_message;
  Endpoint endpoint;
  const bool kReceiveResult =
      config.server->Receive(&received_message, &endpoint);

  // Assert.
  // The receive should have succeeded.
  EXPECT_TRUE(kReceiveResult);
  // It should have received the correct message.
  EXPECT_EQ(kTestMessage.parameter(), received_message.parameter());
  // It should have set the correct endpoint. (The port will be different
  // because this is the port on the server.)
  EXPECT_EQ(endpoint.hostname, kTestEndpoint.hostname);
}

/**
 * @test Tests that receiving from multiple clients works.
 */
TEST(Server, ReceiveMultipleClients) {
  // Arrange.
  auto config = MakeConfig();

  // Send a message to the server.
  const auto kTestMessage = MakeTestMessage();
  ASSERT_TRUE(ConnectAndSend(kTestEndpoint, {kTestMessage}));
  // Send the message again from a new client.
  ASSERT_TRUE(ConnectAndSend(kTestEndpoint, {kTestMessage}));

  // Act.
  // Try receiving the messages.
  TestMessage received_message1, received_message2, received_message3;
  Endpoint endpoint1, endpoint2, endpoint3;
  const bool kReceiveResult1 =
      config.server->Receive(&received_message1, &endpoint1);
  const bool kReceiveResult2 =
      config.server->Receive(&received_message2, &endpoint2);
  const bool kReceiveResult3 =
      config.server->Receive(&received_message3, &endpoint3);

  // Assert.
  // The first receive should have succeeded.
  EXPECT_TRUE(kReceiveResult1);
  // One of the last two should be signalling the disconnect of the first
  // client, so it should have failed. The other should have succeeded.
  EXPECT_NE(kReceiveResult2, kReceiveResult3);

  // Figure out which one contains the message from the second client.
  const auto kSecondClientMessage =
      kReceiveResult2 ? received_message2 : received_message3;
  const auto kSecondClientEndpoint = kReceiveResult2 ? endpoint2 : endpoint3;

  // It should have received the correct messages.
  EXPECT_EQ(kTestMessage.parameter(), received_message1.parameter());
  EXPECT_EQ(kTestMessage.parameter(), kSecondClientMessage.parameter());

  // It should have set the correct endpoints.
  EXPECT_EQ(endpoint1.hostname, kTestEndpoint.hostname);
  EXPECT_EQ(kSecondClientEndpoint.hostname, kTestEndpoint.hostname);
  // The ports should be different because the clients are different.
  EXPECT_NE(endpoint1.port, kSecondClientEndpoint.port);
}

/**
 * @test Tests that we can send a single message.
 */
TEST(Server, SendSingleMessage) {
  // Arrange.
  auto config = MakeConfig();

  // Set up a thread for receiving a message.
  TestResponse got_response;
  std::thread receiver_thread(ConnectAndReceive, kTestEndpoint, &got_response);

  // Wait for the client to register as connected.
  ASSERT_TRUE(WaitForConnection(config));

  // Act.
  // Send a message to the client.
  const auto kTestResponse = MakeTestResponse();
  // Make sure we send to the correct endpoint.
  const auto kSendEndpoint = *config.server->GetConnected().begin();
  const int kSendResult = config.server->Send(kTestResponse, kSendEndpoint);

  // Assert.
  // It should have sent the message successfully.
  EXPECT_GT(kSendResult, 0);

  // The client should have received the message.
  receiver_thread.join();
  EXPECT_EQ(kTestResponse.parameter(), got_response.parameter());
}

/**
 * @test Tests that we can send a single message asynchronously.
 */
TEST(Server, SendSingleMessageAsync) {
  // Arrange.
  auto config = MakeConfig();

  // Set up a thread for receiving a message.
  TestResponse got_response;
  std::thread receiver_thread(ConnectAndReceive, kTestEndpoint, &got_response);

  // Wait for the client to register as connected.
  ASSERT_TRUE(WaitForConnection(config));

  // Act.
  // Send a message to the client.
  const auto kTestResponse = MakeTestResponse();
  // Make sure we send to the correct endpoint.
  const auto kSendEndpoint = *config.server->GetConnected().begin();
  const bool kSendResult =
      config.server->SendAsync(kTestResponse, kSendEndpoint);

  // Assert.
  // It should have sent the message successfully.
  EXPECT_TRUE(kSendResult);

  // The client should have received the message.
  receiver_thread.join();
  EXPECT_EQ(kTestResponse.parameter(), got_response.parameter());
}

/**
 * @test Tests that sending fails when we try so send to a client that's not
 *  connected.
 */
TEST(Server, SendNonexistentClient) {
  // Arrange.
  auto config = MakeConfig();

  // Act.
  const bool kSendResult =
      config.server->SendAsync(MakeTestMessage(), kTestEndpoint);

  // Assert.
  // It should have failed to send.
  EXPECT_FALSE(kSendResult);
}

}  // namespace message_passing::tests
