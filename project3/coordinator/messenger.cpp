/**
 * @file Implementation of the Messenger class.
 */
#include "messenger.h"

#include <chrono>
namespace coordinator::messenger {

Messenger::Messenger(
    std::shared_ptr<queue::Queue<MessageLog::Message>> msg_queue,
    std::shared_ptr<MessageLog> msg_log,
    std::shared_ptr<ConnectedParticipants::Participant> participant,
    std::shared_ptr<ConnectedParticipants> connected_participants)
    : msg_queue_(msg_queue),
      msg_log_(msg_log),
      connected_participants_(connected_participants),
      participant_(participant) {}

bool Messenger::BroadcastMessage() {
  // Pop next message from message queue.
  MessageLog::Message msg = msg_queue_->Pop();

  // Populate protobuf message.
  proto_msg_.set_message(msg.msg);
  proto_msg_.set_origin_id(msg.participant_id);

  // Serialize the message.
  if (!wire_protocol::Serialize(proto_msg_, &outgoing_message_buffer_)) {
    // LOG_F(ERROR, "Failed to serialize message.");
    return false;
  }
  // Send the message to each connected participant.
  for (ConnectedParticipants::Participant participant :
       connected_participants_->GetParticipants()) {
    if (send(participant.sock_fd, outgoing_message_buffer_.data(),
             outgoing_message_buffer_.size(), 0) < 0) {
      // LOG_F(ERROR, "Failed to send message.");
      return false;
    }
  }
  // Update this messages' timestamp.
  const auto p1 = std::chrono::system_clock::now();
  double timestamp = p1.time_since_epoch().count();
  msg.timestamp = timestamp;
  // Add this message to the message log.
  msg_log_->Insert(msg);
  return true;
}

bool Messenger::SendMissedMessages(uint32_t reconnection_time) {
  for (MessageLog::Message msg :
       msg_log_->GetMissedMessages(reconnection_time)) {
    // Populate protobuf message.
    proto_msg_.set_message(msg.msg);
    proto_msg_.set_origin_id(msg.participant_id);

    // Serialize
    if (!wire_protocol::Serialize(proto_msg_, &outgoing_message_buffer_)) {
      return false;
    }

    if (send(participant_->sock_fd, outgoing_message_buffer_.data(),
             outgoing_message_buffer_.size(), 0) < 0) {
      return false;
    }
  }
  return true;
}

}  // namespace coordinator::messenger
