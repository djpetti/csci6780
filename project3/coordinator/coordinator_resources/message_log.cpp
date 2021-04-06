/**
 * @file Implementation of MessageLog.
 */

#include "message_log.h"

#include <algorithm>

namespace coordinator {

bool MessageLog::Message::operator==(const Message &message) const {
  return timestamp == message.timestamp && msg == message.msg &&
         participant_id == message.participant_id;
}

size_t MessageLog::Hash::operator()(const Message &message) const {
  return std::hash<std::string>()(message.msg) ^
         std::hash<int64_t>()(message.timestamp.time_since_epoch().count()) ^
         std::hash<uint32_t>()(message.participant_id);
}

void MessageLog::Insert(const Message &msg) {
  std::lock_guard<std::mutex> guard(mutex_);
  coordinator_messages_.insert(msg);
}

void MessageLog::Clear() {
  std::lock_guard<std::mutex> guard(mutex_);
  coordinator_messages_.clear();
}

std::unordered_set<MessageLog::Message, MessageLog::Hash>
MessageLog::GetMissedMessages(const Timestamp &disconnection_time,
                              const Timestamp &reconnection_time) {
  std::lock_guard<std::mutex> guard(mutex_);

  std::unordered_set<MessageLog::Message, MessageLog::Hash> missed_messages;
  for (const MessageLog::Message &msg : coordinator_messages_) {
    if (msg.timestamp > disconnection_time &&
        (reconnection_time - msg.timestamp) <= td_) {
      missed_messages.insert(msg);
    }
  }
  return missed_messages;
}

MessageLog::MessageLog(const MessageLog::Duration &time_threshold)
    : td_(time_threshold) {}

}  // namespace coordinator