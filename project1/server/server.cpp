#include "server.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <thread>

#include "agent.h"
#include "file_handler/file_handler.h"
#include "../thread_pool/thread_pool.h"

namespace server {
namespace {

using file_handler::FileHandler;

/// Maximum queue size to use for listening on the server socket.
constexpr uint8_t kMaxQueueSize = 5;

/**
 * @brief Helper function that creates the address structure to use for the
 *   server.
 * @param port The port to listen on.
 * @return The address structure.
 */
struct sockaddr_in MakeAddress(uint16_t port) {
  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  return address;
}

/**
 * @brief Sets up the server socket. After this is complete, the socket is
 * ready to accept new connections.
 * @param The address structure to use for configuring the socket.
 * @return The socket FD if successful, or -1 if not.
 */
int SetUpSocket(const struct sockaddr_in &address) {
  // Open a TCP socket.
  const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == 0) {
    perror("Failed to create server socket");
    return -1;
  }

  // Allow the server to re-bind to this port if it was restarted quickly.
  const int option = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option,
                 sizeof(option))) {
    perror("Failed to set socket options");
    // This is not a fatal error.
  }

  // Bind to the port.
  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind() failed on server socket");
    return -1;
  }
  if (listen(server_fd, kMaxQueueSize) < 0) {
    perror("listen() failed on server socket");
    return -1;
  }

  return server_fd;
}

/**
 * @brief Handles a single connection. Meant to be run in a thread.
 * @param client_fd The file descriptor of the client.
 * @return True if the client was successfully serviced.

bool HandleClient(int client_fd) {
  auto file_handler = std::make_unique<FileHandler>();
  Agent agent(client_fd, std::move(file_handler));

  return agent.Handle();
}
*/
}  // namespace

void Server::Listen(uint16_t port) {
  // Create the socket.
  const auto kAddress = MakeAddress(port);

  thread_pool::ThreadPool pool;
  int server_fd = SetUpSocket(kAddress);
  if (server_fd < 0) {
    return;
  }

  std::cout << "Server is listening on port " << port << "." << std::endl;

  while (true) {
    // Accept a new connection.
    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) {
      perror("accept() failed");
      return;
    }

    std::cout << "Handling new connection." << std::endl;

    // Create a new agent and handle the client in a new thread.
    std::thread agent_thread(HandleClient, client_fd);
    // For now, we can get away with just flinging this thread off into the
    // ether.
    agent_thread.detach();
  }
}

}  // namespace server