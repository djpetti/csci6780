/**
 * @file Implementation of coordinator class.
 */
#include "coordinator.h"

#include <loguru.hpp>
#include <random>
namespace coordinator {
using coordinator::ConnectedParticipants;
using pub_sub_messages::CoordinatorMessage;

uint32_t Coordinator::id_ = 0;

Coordinator::Coordinator(int participant_fd, std::string hostname,
                         std::shared_ptr<MessengerManager> msg_mgr,
                         std::shared_ptr<Registrar> registrar)
    : hostname_(hostname),
      msg_mgr_(msg_mgr),
      registrar_(registrar),
      client_fd_(participant_fd) {}

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
  } else if (message.has_deregister()) {
  } else if (message.has_disconnect()) {
  } else if (message.has_reconnect()) {
  } else if (message.has_send_multicast()) {
  }
  LOG_F(ERROR, "No valid message from client (%i) was recieved.", client_fd_);
  return ClientState::ERROR;
}

Coordinator::ClientState Coordinator::HandleRequest(
    const pub_sub_messages::Register &request) {
  ConnectedParticipants::Participant participant;
  participant.id = GenerateID();
  participant.hostname = hostname_;
  participant.port = request.
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

uint32_t Coordinator::GenerateID() {
  id_++;
  return id_;
}
}  // namespace coordinator