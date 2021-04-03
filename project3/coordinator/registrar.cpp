/**
 * @file Implementation of Registrar class.
 */
#include "registrar.h"
#include "connected_participants.h"
#include <cstdint>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
namespace coordinator::registrar {

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
    perror("Socket creation error");
    return -1;
  }

  if (inet_pton(AF_INET, hostname.c_str(),
                (struct sockaddr *)&address.sin_addr) <= 0) {
    perror("Invalid Address or Address not supported");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Connection Failed");
    return -1;
  }

  return sock;
}

Registrar::Registrar(
    std::shared_ptr<ConnectedParticipants> connected_participants)
    : connected_participants_(connected_participants) {}

void Registrar::ConnectParticipant(struct ConnectedParticipants::Participant &participant) {
  connected_participants_->AddParticipant(participant);
}

void Registrar::DisconnectParticipant(struct ConnectedParticipants::Participant &participant) {
  connected_participants_->DeleteParticipant(participant);
}

void Registrar::RegisterParticipant(struct ConnectedParticipants::Participant &participant) {
  // set up socket connection for sending messages to this participant.
  int sock = SetUpSocket(MakeAddress(participant.port), participant.hostname);
  if (sock > 0) {
    participant.sock_fd = sock;
    ConnectParticipant(participant);
  } else {
    // Log socket creation error
  }
}

void Registrar::DeregisterParticipant(struct ConnectedParticipants::Participant &participant) {
  // disconnect participant and close the socket.
  DisconnectParticipant(participant);
  close(participant.sock_fd);
}
}  // namespace coordinator::registrar