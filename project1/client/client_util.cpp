#include "client_util.h"

#include <algorithm>
#include <cerrno>

namespace client_util {
namespace {

/// Timeout in seconds for socket operations.
constexpr uint32_t kSocketTimeout = 1;

}  // namespace

struct sockaddr_in MakeAddress(uint16_t port) {
  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  return address;
}

int SetUpSocket(const sockaddr_in &address, const std::string &hostname) {
  int sock = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation error");
    return -1;
  }

  if (inet_pton(AF_INET, hostname.c_str(),
                (struct sockaddr *)&address.sin_addr) <= 0) {
    perror("Invalid Address or Address not supported");
    return -1;
  }

  // Set a timeout.
  struct timeval timeout {};
  timeout.tv_sec = kSocketTimeout;
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout,
             sizeof(timeout));

  if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Connection Failed");
    return -1;
  }

  return sock;
}

void SaveIncomingFile(const std::string &contents, const std::string &name) {
  std::ofstream new_file;
  new_file.open(name);
  new_file << contents;
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

}  // namespace client_util