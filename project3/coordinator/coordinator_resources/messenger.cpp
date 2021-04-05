/**
 * @file Implementation of the Messenger class.
 */

#include "messenger.h"

#include <chrono>
#include <loguru.hpp>
#include <utility>

namespace coordinator {

Messenger::Messenger(std::shared_ptr<MessageLog> msg_log,
                     ConnectedParticipants::Participant& participant)
    : msg_log_(std::move(msg_log)), participant_(std::move(participant)) {}

const ConnectedParticipants::Participant& Messenger::GetParticipant() const {
  return participant_;
}

bool Messenger::SerializeMessage(const MessageLog::Message &msg) {
  // Populate protobuf message.
  proto_msg_.set_message(msg.msg);
  proto_msg_.set_origin_id(msg.participant_id);

  // Serialize
  if (!wire_protocol::Serialize(proto_msg_, &outgoing_message_buffer_)) {
    return false;
  }
  return true;
}
bool Messenger::SendMessage(const MessageLog::Message &msg) {
  // mutex lock to ensure thread-safe socket sending.
  std::lock_guard<std::mutex> guard(mutex_);
  SerializeMessage(msg);
  if (send(participant_.sock_fd, outgoing_message_buffer_.data(),
           outgoing_message_buffer_.size(), 0) < 0) {
    LOG_F(ERROR, "Failed to send message.");
    return false;
  }
  return true;
}
void Messenger::LogMessage(MessageLog::Message *msg) {
  // Update this messages' timestamp.
  const auto timestamp = std::chrono::steady_clock::now();
  msg->timestamp = timestamp;
  // Add this message to the message log.
  msg_log_->Insert(*msg);
}
bool Messenger::SendMissedMessages(
    const MessageLog::Timestamp& reconnection_time) {
  int msg_count = 0;
  for (MessageLog::Message msg :
       msg_log_->GetMissedMessages(reconnection_time)) {
    if (!SendMessage(msg)) {
      return false;
    }
    msg_count++;
  }
  LOG_F(INFO, "%i missed messages sent to Participant #%i.", msg_count,
        participant_.id);
  return true;
}
}  // namespace coordinator
