/**
 * @file Implementation of CoordinatorDriver
 */
#include "coordinator_driver.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include <chrono>
#include <cstdint>
#include <loguru.hpp>
#include <memory>
namespace coordinator {
using Duration = std::chrono::steady_clock::duration;
struct sockaddr_in CoordinatorDriver::MakeAddress(uint16_t port) {
  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  return address;
}

int CoordinatorDriver::SetUpSocket(const struct sockaddr_in &address) {
  // Open a TCP socket.
  const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    LOG_F(ERROR, "Failed to create server socket");
    return -1;
  }

  // Allow the server to re-bind to this port if it was restarted quickly.
  const int option = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option,
                 sizeof(option))) {
    LOG_F(ERROR, "Failed to set socket options");
    // This is not a fatal error.
  }

  // Bind to the port.
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    LOG_F(ERROR, "bind() failed on server socket");
    return -1;
  }
  if (listen(server_fd, kMaxQueueSize_) < 0) {
    LOG_F(ERROR, "listen() failed on server socket");
    return -1;
  }

  return server_fd;
}

int CoordinatorDriver::CreateSocket(uint16_t port) {
  // Create the socket.
  const auto kAddress = MakeAddress(port);
  return SetUpSocket(kAddress);
}

[[noreturn]] void CoordinatorDriver::Start(uint16_t port) {
  int server_fd = CreateSocket(port);
  participants_ = std::make_shared<coordinator::ConnectedParticipants>();
  messenger_manager_ = std::make_shared<MessengerManager>(participants_);
  registrar_ = std::make_shared<Registrar>(participants_);
  Duration time;
  message_log_ = std::make_shared<MessageLog>(time);
  while (true) {
    struct sockaddr_in client_address;
    socklen_t size = sizeof(client_address);
    // Accept a new connection.
    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_address, &size);
    if (client_fd < 0) {
      LOG_F(ERROR, "accept() failed");
    }
    LOG_F(INFO, "Deploying coordinator task for client #%i.", client_fd);
    std::string hostname(inet_ntoa(client_address.sin_addr));

    auto coordinator_task = std::make_shared<coordinator::CoordinatorTask>(
        client_fd, hostname, messenger_manager_, registrar_, message_queue_,
        message_log_);
    pool_.AddTask(coordinator_task);
  }
}
}  // namespace coordinator
