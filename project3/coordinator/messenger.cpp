/**
 * @file Implementation of the Messenger class.
 */

#include "messenger.h"

#include <chrono>
#include <loguru.hpp>
#include <utility>

namespace coordinator {

Messenger::Messenger(std::shared_ptr<MessageLog> msg_log,
                     ConnectedParticipants::Participant participant)
    : msg_log_(std::move(msg_log)), participant_(std::move(participant)) {}
bool Messenger::operator==(const Messenger& messenger) const {
  return participant_ == messenger.participant_;
}

ConnectedParticipants::Participant Messenger::GetParticipant() const {
  return participant_;
}

bool Messenger::SerializeMessage(MessageLog::Message msg) {
  // Populate protobuf message.
  proto_msg_.set_message(msg.msg);
  proto_msg_.set_origin_id(msg.participant_id);

  // Serialize
  if (!wire_protocol::Serialize(proto_msg_, &outgoing_message_buffer_)) {
    return false;
  }
  return true;
}
bool Messenger::SendMessage(MessageLog::Message msg, bool missed_msg) {
  SerializeMessage(msg);
  // mutex lock to ensure thread-safe socket sending.
  std::lock_guard<std::mutex> guard(mutex_);
  if (send(participant_.sock_fd, outgoing_message_buffer_.data(),
           outgoing_message_buffer_.size(), 0) < 0) {
    LOG_F(ERROR, "Failed to send message.");
    return false;
  }

  if(!missed_msg) {
    // Update this messages' timestamp.
    const auto timestamp = std::chrono::steady_clock::now();
    msg.timestamp = timestamp;
    // Add this message to the message log.
    msg_log_->Insert(msg);
  }

  return true;
}

bool Messenger::SendMissedMessages(
    const MessageLog::Timestamp& reconnection_time) {
  for (const MessageLog::Message& msg :
       msg_log_->GetMissedMessages(reconnection_time)) {
    if (!SendMessage(msg, true)) {
      return false;
    }
  }
  return true;
}
}  // namespace coordinator
