/**
 * @file Implementation of coordinator class.
 */
#include "coordinator.h"

#include <chrono>
#include <loguru.hpp>
#include <utility>
namespace coordinator {
using coordinator::ConnectedParticipants;
using pub_sub_messages::CoordinatorMessage;
using Timestamp = std::chrono::steady_clock::time_point;

uint32_t Coordinator::id_ = 0;

Coordinator::Coordinator(
    int participant_fd, std::string hostname,
    std::shared_ptr<MessengerManager> msg_mgr,
    std::shared_ptr<Registrar> registrar,
    std::shared_ptr<queue::Queue<MessageLog::Message>> msg_queue,
    std::shared_ptr<MessageLog> msg_log)
    : msg_queue_(std::move(msg_queue)),
      msg_log_(std::move(msg_log)),
      client_fd_(participant_fd),
      hostname_(std::move(hostname)),
      messenger_mgr_(std::move(msg_mgr)),
      registrar_(std::move(registrar)) {}

bool Coordinator::Coordinate() {
  ClientState client_state = ClientState::ACTIVE;

  // Get the message from the socket.
  CoordinatorMessage message;
  client_state = ReadNextMessage(&message);

  if (client_state == ClientState::ACTIVE) {
    // Handle the message.
    client_state = DispatchMessage(message);
  }

  return client_state != ClientState::ERROR;
}

ConnectedParticipants::Participant Coordinator::DetermineParticipant(
    uint32_t id) {
  // determine the participant.
  for (auto participant :
       registrar_->GetConnectedParticipants()->GetParticipants()) {
    if (participant.id == id) {
      return participant;
    }
  }
  LOG_F(ERROR, "Error determining participant for client.");
  return ConnectedParticipants::Participant();
}
std::shared_ptr<Messenger> Coordinator::DetermineMessenger(uint32_t id) {
  // determine this participant's messenger.
  for (auto messenger : messenger_mgr_->GetMessengers()) {
    if (messenger->GetParticipant().id == id) {
      return messenger;
    }
  }
  LOG_F(ERROR, "Error determining messenger for client.");
  return std::shared_ptr<Messenger>();
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
  ConnectedParticipants::Participant participant;
  participant.id = GenerateID();
  participant.hostname = hostname_;
  participant.port = request.port_number();
  LOG_F(INFO, "Handling a register command from Participant %i.",
        participant.id);

  registrar_->RegisterParticipant(participant);
  // initialize this participant's Messenger and register it with the messenger
  // manager.
  auto messenger = std::make_shared<Messenger>(msg_log_, participant);
  messenger_mgr_->AddMessenger(messenger);

  pub_sub_messages::RegistrationResponse response;
  SendRegistrationResponse(response, participant);
  return ClientState::ACTIVE;
}
Coordinator::ClientState Coordinator::HandleRequest(
    const pub_sub_messages::Deregister &request) {
  DetermineParticipant(request.participant_id());
  DetermineMessenger(request.participant_id());
  LOG_F(INFO, "Handling a deregister command from Participant %i.",
        request.participant_id());
  auto participant = DetermineParticipant(request.participant_id());
  auto messenger = DetermineMessenger(request.participant_id());
  registrar_->DeregisterParticipant(participant);
  messenger_mgr_->DeleteMessenger(messenger);
  return ClientState::ACTIVE;
}

Coordinator::ClientState Coordinator::HandleRequest(
    const pub_sub_messages::Disconnect &request) {
  DetermineParticipant(request.participant_id());
  LOG_F(INFO, "Handling a disconnect command from Participant %i.",
        request.participant_id());
  auto participant = DetermineParticipant(request.participant_id());
  registrar_->DisconnectParticipant(participant);
  return ClientState::ACTIVE;
}

Coordinator::ClientState Coordinator::HandleRequest(
    const pub_sub_messages::Reconnect &request) {
  DetermineParticipant(request.participant_id());
  DetermineMessenger(request.participant_id());
  LOG_F(INFO, "Handling a reconnect command from Participant %i.",
        request.participant_id());
  auto participant = DetermineParticipant(request.participant_id());
  auto messenger = DetermineMessenger(request.participant_id());
  // reconnect this participant via registrar.
  registrar_->ConnectParticipant(participant);
  // send any missed messages satisfying the time threshold.
  Timestamp timestamp = std::chrono::steady_clock::now();
  messenger->SendMissedMessages(timestamp);
  return ClientState::ACTIVE;
}

Coordinator::ClientState Coordinator::HandleRequest(
    const pub_sub_messages::SendMulticast &request) {
  DetermineParticipant(request.participant_id());
  LOG_F(INFO, "Handling a msend command from Participant %i.",
        request.participant_id());
  auto participant = DetermineParticipant(request.participant_id());
  // create and broadcast the message.
  MessageLog::Message message;
  Timestamp timestamp;
  message.timestamp = timestamp;
  message.msg = request.message();
  message.participant_id = participant.id;
  // ensures that messages get sent in the order they were received.
  msg_queue_->Push(message);
  coordinator::MessageLog::Message msg = msg_queue_->Pop();
  if (!messenger_mgr_->BroadcastMessage(&msg)) {
    LOG_F(ERROR, "Error broadcasting this message from participant (%i)",
          participant.id);
  }
  return ClientState::ACTIVE;
}
uint32_t Coordinator::GenerateID() {
  bool duplicate = false;
  do {
    id_++;
    // ensure this ID is not already taken.
    for (const auto& participant :
         registrar_->GetConnectedParticipants()->GetParticipants()) {
      if (participant.id == id_) {
        duplicate = true;
        break;
      } else {
        duplicate = false;
      }
    }
  } while (duplicate);

  return id_;
}
bool Coordinator::SendRegistrationResponse(
    pub_sub_messages::RegistrationResponse &response,
    const ConnectedParticipants::Participant& participant) {
  response.set_participant_id(participant.id);
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