/**
 * @file Implementation of coordinator class.
 */
#include "coordinator.h"

#include <loguru.hpp>
#include <chrono>
namespace coordinator {
using coordinator::ConnectedParticipants;
using pub_sub_messages::CoordinatorMessage;
/// Type alias for timestamps.
using Timestamp = std::chrono::steady_clock::time_point;

uint32_t Coordinator::id_ = 0;

Coordinator::Coordinator(
    int participant_fd, std::string hostname,
    std::shared_ptr<MessengerManager> msg_mgr,
    std::shared_ptr<Registrar> registrar,
    std::shared_ptr<queue::Queue<MessageLog::Message>> msg_queue,
    std::shared_ptr<MessageLog> msg_log)
    : msg_queue_(msg_queue),
      msg_log_(msg_log),
      client_fd_(participant_fd),
      hostname_(hostname),
      msg_mgr_(msg_mgr),
      registrar_(registrar){}

bool Coordinator::Coordinate() {
  ClientState client_state = ClientState::ACTIVE;

  while (client_state == ClientState::ACTIVE) {
    // Get the next message from the socket.
    CoordinatorMessage message;
    client_state = ReadNextMessage(&message);

    if (client_state == ClientState::ACTIVE) {
      // Handle the message.
      client_state = DispatchMessage(message);
    }
  }

  return client_state != ClientState::ERROR;
}

Coordinator::ClientState Coordinator::ReadNextMessage(
    pub_sub_messages::CoordinatorMessage *message) {
  while (!parser_.HasCompleteMessage()) {
    incoming_message_buffer_.resize(kClientBufferSize);

    // Read some more data from the socket.
    const auto bytes_read =
        recv(client_fd_, incoming_message_buffer_.data(), kClientBufferSize, 0);

    if (bytes_read < 0) {
      // Failed to read anything.
      LOG_F(ERROR, "Failed to read from client (%i) socket.", client_fd_);
      parser_.ResetParser();
      return ClientState::ERROR;
    } else if (bytes_read == 0) {
      // Client has disconnected nicely.
      LOG_F(INFO, "Client with FD %i has disconnected.", client_fd_);
      return ClientState::DISCONNECTED;
    }

    // The parser assumes that the entire vector contains valid data, so limit
    // the size.
    incoming_message_buffer_.resize(bytes_read);

    // Add the data to the parser.
    parser_.AddNewData(incoming_message_buffer_);
  }

  // Get the parsed message.
  if (!parser_.GetMessage(message)) {
    LOG_F(ERROR, "Failed to get the parsed message from client (%i).",
          client_fd_);
    return ClientState::ERROR;
  }
  return ClientState::ACTIVE;
}

Coordinator::ClientState Coordinator::DispatchMessage(
    const pub_sub_messages::CoordinatorMessage &message) {
  if (message.has_register_()) {
    return HandleRequest(message.register_());
  } else if (message.has_deregister()) {
    return HandleRequest(message.deregister());
  } else if (message.has_disconnect()) {
    return HandleRequest(message.disconnect());
  } else if (message.has_reconnect()) {
    return HandleRequest(message.reconnect());
  } else if (message.has_send_multicast()) {
    return HandleRequest(message.send_multicast());
  }
  LOG_F(ERROR, "No valid message from client (%i) was recieved.", client_fd_);
  return ClientState::ERROR;
}

Coordinator::ClientState Coordinator::HandleRequest(
    const pub_sub_messages::Register &request) {
  // create this participant and register with the registrar.
  participant_.id = request.participant_id();
  participant_.hostname = hostname_;
  participant_.port = request.port_number();
  registrar_->RegisterParticipant(participant_);
  // initialize this participant's Messenger and register it with the messenger manager.
  messenger_ = std::make_shared<Messenger>(msg_log_,participant_);
  msg_mgr_->AddMessenger(messenger_);
  //pub_sub_messages::RegistrationResponse response;
  //SendRegistrationResponse(response);
  return ClientState::ACTIVE;
}
Coordinator::ClientState Coordinator::HandleRequest(
    const pub_sub_messages::Deregister &request) {
  registrar_->DeregisterParticipant(participant_);
  return ClientState::ACTIVE;
}

Coordinator::ClientState Coordinator::HandleRequest(
    const pub_sub_messages::Disconnect &request) {
  registrar_->DisconnectParticipant(participant_);
  return ClientState::ACTIVE;
}

Coordinator::ClientState Coordinator::HandleRequest(
    const pub_sub_messages::Reconnect &request) {
  // reconnect this participant via registrar.
  registrar_->ConnectParticipant(participant_);
  // send any missed messages satisfying the time threshold.
  Timestamp timestamp;
  messenger_->SendMissedMessages(timestamp);
  return ClientState::ACTIVE;
}

Coordinator::ClientState Coordinator::HandleRequest(const pub_sub_messages::SendMulticast &request) {
  MessageLog::Message message;
  // create and broadcast the message.
  Timestamp timestamp;
  message.timestamp = timestamp;
  message.msg = request.message();
  message.participant_id = participant_.id;
  msg_queue_->Push(message);
  if (!msg_mgr_->BroadcastMessage(msg_queue_->Pop())){
    LOG_F(ERROR, "Error broadcasting this message from participant (%i)",participant_.id);
  }
  return ClientState::ACTIVE;
}

bool Coordinator::SendRegistrationResponse(
    const pub_sub_messages::RegistrationResponse &response) {
  // Serialize the message.
  if (!wire_protocol::Serialize(response, &outgoing_message_buffer_)) {
    LOG_F(ERROR, "Failed to serialize message.");
    return false;
  }

  // Send the message.
  if (send(client_fd_, outgoing_message_buffer_.data(),
           outgoing_message_buffer_.size(), 0) < 0) {
    LOG_F(ERROR, "Failed to send message.");
    return false;
  }
  return true;
}

}  // namespace coordinator