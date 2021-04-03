/**
 * @file Implementation of MessageLog.
 */
#include "message_log.h"
namespace coordinator::message_log {

bool MessageLog::Message::operator==(const Message &message) const {
  return timestamp == message.timestamp && msg == message.msg &&
         participant_id == message.participant_id;
}

size_t MessageLog::Hash::operator()(const Message &message) const {
  // ensure uniqueness from this messages length and time stamp
  return std::hash<int>()(message.msg.length() + message.timestamp);
}

void MessageLog::Insert(struct Message &msg) {
  std::lock_guard<std::mutex> guard(mutex_);
  coordinator_messages_.insert(msg);
}
void MessageLog::Clear() {
  std::lock_guard<std::mutex> guard(mutex_);
  coordinator_messages_.clear();
}

std::unordered_set<MessageLog::Message, MessageLog::Hash>
MessageLog::GetMissedMessages(uint32_t reconnection_time) {
  std::unordered_set<MessageLog::Message, MessageLog::Hash> missed_messages;
  for (MessageLog::Message msg : coordinator_messages_) {
    if ((reconnection_time - msg.timestamp) <= td_) {
      missed_messages.insert(msg);
    }
  }
  return missed_messages;
}

}  // namespace coordinator::message_log