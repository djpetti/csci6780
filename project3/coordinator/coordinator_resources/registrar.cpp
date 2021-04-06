/**
 * @file Implementation of Registrar class.
 */
#include "registrar.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <loguru.hpp>
#include <string>
#include <thread>
#include <utility>

#include "participant_manager.h"

namespace coordinator {
namespace {

/// Time to sleep between connection retries.
const std::chrono::seconds kRetryWaitTime(1);
/// Maximum number of times to retry the connection.
const uint32_t kMaxRetryTimes = 3;

struct sockaddr_in MakeAddress(uint16_t port) {
  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  return address;
}

/**
 * @brief Connects to a socket.
 * @param address The address to connect to.
 * @param hostname The associated hostname.
 * @return The socket fd, or -1 on failure.
 */
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
    LOG_S(ERROR) << "Connection to " << hostname
                 << " Failed: " << std::strerror(errno);
    return -1;
  }

  return sock;
}

/**
 * @brief Connects to a socket, retrying multiple times if it fails.
 * @param address The address to connect to.
 * @param hostname The associated hostname.
 * @param max_retries The maximum number of times to retry.
 * @return The socket fd, or -1 on failure.
 */
int SetUpSocketRetry(const sockaddr_in &address, const std::string &hostname,
                     uint32_t max_retries) {
  uint32_t num_retries = 0;
  int socket = -1;

  while (num_retries++ < max_retries &&
         (socket = SetUpSocket(address, hostname)) < 0) {
    LOG_S(WARNING) << "Socket connection failed, will retry.";
    std::this_thread::sleep_for(kRetryWaitTime);
  }

  return socket;
}

/**
 * @brief Connects to a participant socket.
 * @param participant The participant to connect to. Socket will be updated.
 * @return True if it successfully connected.
 */
bool ConnectToParticipant(ParticipantManager::Participant *participant) {
  // set up socket connection for sending messages to this participant.
  int sock = SetUpSocketRetry(MakeAddress(participant->port),
                              participant->hostname, kMaxRetryTimes);
  if (sock > 0) {
    LOG_F(INFO, "Participant #%i has been registered.", participant->id);
    participant->sock_fd = sock;
    return true;
  } else {
    LOG_S(ERROR) << "Failed to connect to participant.";
    return false;
  }
}

}  // namespace

Registrar::Registrar(std::shared_ptr<ParticipantManager> participants)
    : participants_(std::move(participants)) {}

void Registrar::ReconnectParticipant(
    ParticipantManager::Participant *participant) {
  LOG_S(INFO) << "Participant " << participant->id << " reconnected";

  if (ConnectToParticipant(participant)) {
    participants_->ReconnectParticipant(*participant);
  }
}

void Registrar::DisconnectParticipant(
    const ParticipantManager::Participant &participant) {
  LOG_S(INFO) << "Participant " << participant.id << " disconnected.";

  participants_->DisconnectParticipant(participant);
}

void Registrar::RegisterParticipant(
    ParticipantManager::Participant *participant) {
  if (ConnectToParticipant(participant)) {
    participants_->RegisterParticipant(*participant);
  }
}

void Registrar::DeregisterParticipant(
    struct ParticipantManager::Participant &participant) {
  // disconnect participant and close the socket.
  LOG_F(INFO, "Participant #%i has been deregistered.", participant.id);
  participants_->DeregisterParticipant(participant);
  close(participant.sock_fd);
}

ParticipantManager *Registrar::GetParticipants() {
  ParticipantManager *p = participants_.get();
  return p;
}

}  // namespace coordinator