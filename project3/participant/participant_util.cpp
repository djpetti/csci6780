#include "participant_util.h"

#include <algorithm>
#include <cerrno>
#include <loguru.hpp>

namespace participant_util {
namespace {

constexpr uint8_t kMaxListenerSize = 5;

}  // namespace

struct sockaddr_in MakeAddress(uint16_t port) {
  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  return address;
}

int SetUpListenerSocket(const struct sockaddr_in &address) {
  // Open a TCP socket.
  const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    LOG_S(FATAL) << "Failed to create server socket";
    return -1;
  }

  // Allow the server to re-bind to this port if it was restarted quickly.
  const int option = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option,
                 sizeof(option))) {
    LOG_S(FATAL) << "Failed to set socket options";
    // This is not a fatal error.
  }

  // Bind to the port.
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    LOG_S(FATAL) << "bind() failed on server socket";
    return -1;
  }
  if (listen(server_fd, kMaxListenerSize) < 0) {
    LOG_S(FATAL) << "listen() failed on server socket";
    return -1;
  }

  return server_fd;
}

int SetUpSocket(const sockaddr_in &address, const std::string &hostname) {
  int sock = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    LOG_S(FATAL) << "Socket creation error";
    return -1;
  }

  if (inet_pton(AF_INET, hostname.c_str(),
                (struct sockaddr *)&address.sin_addr) <= 0) {
    LOG_S(FATAL) << "Invalid Address or Address not supported";
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    LOG_S(FATAL) << "Connection Failed";
    return -1;
  }

  return sock;
}

int ReceiveForever(int socket, void *buffer, size_t length, int flags) {
  while (true) {
    const int kRecvResult = recv(socket, buffer, length, flags);

    if (!(kRecvResult == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))) {
      // It didn't time out. Exit.
      return kRecvResult;
    }
  }
}

int SendForever(int socket, const void *buffer, size_t length, int flags) {
  uint32_t total_bytes_sent = 0;

  while (total_bytes_sent < length) {
    const int kSendResult =
        send(socket, static_cast<const uint8_t *>(buffer) + total_bytes_sent,
             length - total_bytes_sent, flags);

    if ((kSendResult == -1 && !(errno == EAGAIN || errno == EWOULDBLOCK)) ||
        kSendResult == 0) {
      // It failed or disconnected, but didn't time out.
      return kSendResult;
    }

    total_bytes_sent += kSendResult;
  }

  return total_bytes_sent;
}

}  // namespace participant_util
