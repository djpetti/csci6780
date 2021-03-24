/**
 * @file Unit tests for `wire_protocol`.
 */

#include <algorithm>
#include <cstdint>
#include <vector>

#include "../wire_protocol.h"
#include "test_messages.pb.h"
#include "gtest/gtest.h"

namespace wire_protocol::tests {

using test_messages::TestMessage;

namespace {

/// Parameter string to use for test messages.
const char* kTestParameterString = "a parameter string value";

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
 * @brief Fixture to use for tests that split the message at different points.
 */
class WireProtocolSplitMessage : public ::testing::TestWithParam<uint32_t> {};

}  // namespace

/**
 * @test Tests that we can serialize a message and then deserialize it
 * successfully.
 */
TEST(WireProtocol, RoundTrip) {
  // Arrange.
  // Create a message.
  const auto kTestMessage = MakeTestMessage();

  // Parser to deserialize with.
  MessageParser<TestMessage> parser;

  std::vector<uint8_t> serialized;

  // Act.
  const bool kSerializedSuccess = Serialize(kTestMessage, &serialized);
  parser.AddNewData(serialized);

  // Assert.
  // The serialization should have succeeded.
  ASSERT_TRUE(kSerializedSuccess);
  // The parser should indicate that the message is complete.
  EXPECT_TRUE(parser.HasCompleteMessage());

  // We should read the correct message.
  TestMessage got_message;
  EXPECT_TRUE(parser.GetMessage(&got_message));
  EXPECT_STREQ(kTestParameterString, got_message.parameter().c_str());
}

/**
 * @test Tests that the parser can handle a message that has been split up.
 */
TEST_P(WireProtocolSplitMessage, RoundTripPartialMessage) {
  // Arrange.
  // Create a message.
  const auto kTestMessage = MakeTestMessage();

  // Serialize the message.
  std::vector<uint8_t> serialized;
  ASSERT_TRUE(Serialize(kTestMessage, &serialized));

  // Split the serialized message into two parts.
  const auto kSplitAt = GetParam();
  std::vector<uint8_t> serialized_part_1(serialized.begin(),
                                         serialized.begin() + kSplitAt);
  std::vector<uint8_t> serialized_part_2(serialized.begin() + kSplitAt,
                                         serialized.end());

  // Act.
  MessageParser<TestMessage> parser;
  parser.AddNewData(serialized_part_1);
  const bool kHasMessageAfterFirstPart = parser.HasCompleteMessage();
  parser.AddNewData(serialized_part_2);

  // Assert.
  // It should not have a complete message until the second part.
  EXPECT_FALSE(kHasMessageAfterFirstPart);
  EXPECT_TRUE(parser.HasCompleteMessage());

  // We should read the correct message.
  TestMessage got_message;
  EXPECT_TRUE(parser.GetMessage(&got_message));
  EXPECT_STREQ(kTestParameterString, got_message.parameter().c_str());
}

INSTANTIATE_TEST_SUITE_P(
    SplitMessageTests, WireProtocolSplitMessage,
    // Last value here splits in the middle of the message data.
    ::testing::Values(1, 4, 4 + MakeTestMessage().ByteSizeLong() / 2));

/**
 * @test Tests that we can parse multiple messages when they arrive together.
 */
TEST(WireProtocol, RoundTripMultiMessage) {
  // Arrange.
  // Create messages.
  const auto kTestMessage1 = MakeTestMessage();
  const auto kTestMessage2 = MakeTestMessage();

  // Serialize the messages.
  std::vector<uint8_t> serialized_1;
  ASSERT_TRUE(Serialize(kTestMessage1, &serialized_1));
  std::vector<uint8_t> serialized_2;
  ASSERT_TRUE(Serialize(kTestMessage2, &serialized_2));

  // Combine into one data stream.
  std::vector<uint8_t> combined(serialized_1.size() + serialized_2.size());
  std::copy(serialized_1.begin(), serialized_1.end(), combined.begin());
  std::copy(serialized_2.begin(), serialized_2.end(),
            combined.begin() + serialized_1.size());

  // Act.
  MessageParser<TestMessage> parser;
  parser.AddNewData(combined);

  // Assert.
  // It should have a complete message.
  EXPECT_TRUE(parser.HasCompleteMessage());

  // The first message should be intact.
  TestMessage got_message;
  EXPECT_TRUE(parser.GetMessage(&got_message));
  EXPECT_STREQ(kTestParameterString, got_message.parameter().c_str());

  // It should still have the second message.
  EXPECT_TRUE(parser.HasCompleteMessage());

  // The second message should be intact.
  EXPECT_TRUE(parser.GetMessage(&got_message));
  EXPECT_STREQ(kTestParameterString, got_message.parameter().c_str());

  // It should have no more messages.
  EXPECT_FALSE(parser.HasCompleteMessage());
}

}  // namespace wire_protocol::tests
