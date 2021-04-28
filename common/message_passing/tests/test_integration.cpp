/**
 * @file Tests that exercise the server and client together.
 */

#include <gtest/gtest.h>

#include <cstdint>
#include <functional>
#include <loguru.hpp>
#include <memory>
#include <utility>

#include "../client.h"
#include "../server.h"
#include "test_messages.pb.h"
#include "thread_pool/thread_pool.h"

namespace message_passing::tests {
namespace {

using test_messages::TestMessage;
using test_messages::TestResponse;
using thread_pool::ThreadPool;

/// Listening port to use for the server.
constexpr uint16_t kServerPort = 1234;
/// Parameter string to use for test messages.
const char* kTestParameterString = "a parameter string value";
/// Server endpoint to use for testing.
const Endpoint kTestEndpoint = {"127.0.0.1", kServerPort};
/// How many times to try connecting before we give up.
constexpr uint8_t kConnectionRetries = 5;

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
  /// The server under test.
  std::unique_ptr<Server> server;
  /// The client under test.
  std::unique_ptr<Client> client;
};

/**
 * @brief Creates standard configuration for tests.
 * @return The configuration that it created.
 */
ConfigForTests MakeConfig() {
  auto thread_pool = std::make_shared<ThreadPool>();
  auto server = std::make_unique<Server>(thread_pool, kServerPort);
  auto client = std::make_unique<Client>(thread_pool, kTestEndpoint);

  return {thread_pool, std::move(server), std::move(client)};
}

using RetryFunction = std::function<bool()>;

/**
 * @brief Allows a function to be run with retries.
 * @param to_retry The function to retry. Should return a boolean indicating
 *  whether it succeeded.
 * @return True if it succeeded within retry limit, false otherwise.
 */
bool Retry(const RetryFunction& to_retry) {
  for (uint8_t num_retries = 0; num_retries < kConnectionRetries;
       ++num_retries) {
    if (to_retry()) {
      // Succeeded. We can stop now.
      return true;
    }

    // Wait before trying again.
    LOG_S(WARNING) << "Waiting before retry.";
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return false;
}

/**
 * @test Tests that we can send a request from the client,
 * and produce a response from the server.
 */
TEST(MessagePassingIntegration, RequestResponse) {
  // Arrange.
  auto config = MakeConfig();

  // Act.
  // Send a request to the server.
  const auto kTestRequest = MakeTestMessage();
  ASSERT_TRUE(Retry([&]() { return config.client->Send(kTestRequest) > 0; }));

  // Receive the request.
  TestMessage got_request;
  ASSERT_TRUE(config.server->Receive(&got_request));

  // Send the response.
  const auto kTestResponse = MakeTestResponse();
  const auto kEndpoints = config.server->GetConnected();
  ASSERT_EQ(1U, kEndpoints.size());
  const auto kClientEndpoint = *kEndpoints.begin();
  ASSERT_GT(config.server->Send(kTestResponse, kClientEndpoint), 0);

  // Receive the response.
  TestResponse got_response;
  Endpoint got_server_endpoint;
  // Any failed sends are going to cause spurious error-status messages to
  // be received, so we need to clear those before we can receive the real one.
  ASSERT_TRUE(Retry([&]() {
    return config.client->Receive(&got_response, &got_server_endpoint);
  }));

  // Assert.
  // The received request should match what was sent.
  EXPECT_EQ(kTestRequest.parameter(), got_request.parameter());
  // The received response should match what was sent.
  EXPECT_EQ(kTestResponse.parameter(), got_response.parameter());

  // The client endpoint should be correct.
  EXPECT_EQ("127.0.0.1", kClientEndpoint.hostname);
  // The server endpoint should be correct.
  EXPECT_EQ(got_server_endpoint, kTestEndpoint);
}

}  // namespace
}  // namespace message_passing::tests