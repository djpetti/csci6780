/**
 * @file Tests for the `MessageLog` class.
 */

#include <chrono>

#include "../message_log.h"
#include "gtest/gtest.h"

namespace coordinator::tests {
namespace {

/// Message retention threshold to use for testing.
const MessageLog::Duration kRetentionThreshold = std::chrono::milliseconds(10);
/// Test message to use for various functions.
const MessageLog::Message kTestMessage{"hello",
                                       std::chrono::steady_clock::now(), 42};

}  // namespace

/**
 * @test Tests that message equality works correctly.
 */
TEST(MessageLog, MessageEquality) {
  // Arrange.
  MessageLog::Message message2 = kTestMessage;

  // Act.
  // They should compare as equal.
  EXPECT_EQ(kTestMessage, message2);

  // Modifying them should change that.
  message2.msg = "goodbye";
  // Have to do it this way because we technically don't implement operator!=.
  EXPECT_FALSE(kTestMessage == message2);
}

/**
 * @test Tests that message hashing works correctly.
 */
TEST(MessageLog, MessageHash) {
  // Arrange.
  MessageLog::Message message2 = kTestMessage;

  // Act.
  // The hashes should be the same.
  EXPECT_EQ(MessageLog::Hash()(kTestMessage), MessageLog::Hash()(message2));

  // Modifying them should change that.
  message2.msg = "goodbye";
  EXPECT_NE(MessageLog::Hash()(kTestMessage), MessageLog::Hash()(message2));
}

/**
 * @test Tests that we can get missed messages.
 */
TEST(MessageLog, TestMissedMessages) {
  // Arrange.
  // Create an older message that should be dropped.
  auto old_message = kTestMessage;
  old_message.timestamp -= kRetentionThreshold * 2;

  // Insert both messages.
  MessageLog log(kRetentionThreshold);
  log.Insert(kTestMessage);
  log.Insert(old_message);

  // Act.
  auto missed_messages =
      log.GetMissedMessages(old_message.timestamp, kTestMessage.timestamp);
  // However, it should give us nothing if the disconnect time is too recent.
  auto missed_messages_fast_reconnect =
      log.GetMissedMessages(kTestMessage.timestamp, kTestMessage.timestamp);

  // Assert.
  // It should have gotten the new message but not the old one.
  EXPECT_EQ(1U, missed_messages.size());
  EXPECT_NE(missed_messages.find(kTestMessage), missed_messages.end());

  // For the instance reconnect case, it should have given us nothing.
  EXPECT_TRUE(missed_messages_fast_reconnect.empty());
}

}  // namespace coordinator::tests
