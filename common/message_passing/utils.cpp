#include "utils.h"

#include <arpa/inet.h>

#include <cerrno>
#include <cstring>
#include <loguru.hpp>

namespace message_passing {
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

int SetUpSocket(const sockaddr_in& address, const std::string& hostname) {
  int sock;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    LOG_S(ERROR) << "Socket creation error: " << std::strerror(errno);
    return -1;
  }

  if (inet_pton(AF_INET, hostname.c_str(),
                (struct sockaddr*)&address.sin_addr) <= 0) {
    LOG_S(ERROR) << "inet_pton: " << std::strerror(errno);
    return -1;
  }

  // Set a timeout.
  struct timeval timeout {};
  timeout.tv_sec = kSocketTimeout;
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,
             sizeof(timeout));

  if (connect(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
    LOG_S(ERROR) << "Connection Failed: " << std::strerror(errno);
    return -1;
  }

  return sock;
}

int SetUpListenerSocket(const struct sockaddr_in &address) {
  // Open a TCP socket.
  const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    LOG_S(ERROR) << "Failed to create server socket";
    return -1;
  }

  // Allow the server to re-bind to this port if it was restarted quickly.
  const int option = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option,
                 sizeof(option))) {
    LOG_S(ERROR) << "Failed to set socket options";
    // This is not a fatal error.
  }

  // Bind to the port.
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    LOG_S(ERROR) << "bind() failed on server socket";
    return -1;
  }
  if (listen(server_fd, 1) < 0) {
    LOG_S(ERROR) << "listen() failed on server socket";
    return -1;
  }

  return server_fd;
}

}  // namespace message_passing