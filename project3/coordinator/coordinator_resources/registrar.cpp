/**
 * @file Implementation of Registrar class.
 */
#include "registrar.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <loguru.hpp>
#include <string>
#include <utility>

#include "connected_participants.h"

namespace coordinator {

struct sockaddr_in MakeAddress(uint16_t port) {
  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  return address;
}

int SetUpSocket(const sockaddr_in &address, const std::string &hostname) {
  int sock;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    LOG_S(ERROR) << "Socket creation error: " << std::strerror(errno);
    return -1;
  }

  if (inet_pton(AF_INET, hostname.c_str(),
                (struct sockaddr *)&address.sin_addr) <= 0) {
    LOG_S(ERROR) << "Invalid Address or Address not supported: "
                 << std::strerror(errno);
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    LOG_S(ERROR) << "Connection Failed: " << std::strerror(errno);
    return -1;
  }

  return sock;
}

Registrar::Registrar(
    std::shared_ptr<ConnectedParticipants> connected_participants)
    : connected_participants_(std::move(connected_participants)) {}

void Registrar::ConnectParticipant(
    const ConnectedParticipants::Participant &participant) {
  LOG_S(INFO) << "Participant " << participant.id << " connected";

  connected_participants_->AddParticipant(participant);
}

void Registrar::DisconnectParticipant(
    const ConnectedParticipants::Participant &participant) {
  LOG_S(INFO) << "Participant " << participant.id << " disconnected.";

  connected_participants_->DeleteParticipant(participant);
}

void Registrar::RegisterParticipant(
    struct ConnectedParticipants::Participant &participant) {
  // set up socket connection for sending messages to this participant.
  int sock = SetUpSocket(MakeAddress(participant.port), participant.hostname);
  if (sock > 0) {
    LOG_F(INFO,"Participant #%i has been registered.", participant.id);
    participant.sock_fd = sock;
    ConnectParticipant(participant);
  } else {
    LOG_S(ERROR) << "Failed to connect to participant.";
  }
}

void Registrar::DeregisterParticipant(
    struct ConnectedParticipants::Participant &participant) {
  // disconnect participant and close the socket.
  LOG_F(INFO,"Participant #%i has been deregistered.", participant.id);
  DisconnectParticipant(participant);
  close(participant.sock_fd);
}

ConnectedParticipants* Registrar::GetConnectedParticipants() {
  ConnectedParticipants *p = connected_participants_.get();
  return p;
}

}  // namespace coordinator