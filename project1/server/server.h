/**
 * @file Main class for the server.
 */

#ifndef PROJECT1_SERVER_H
#define PROJECT1_SERVER_H

#include <cstdint>
#include "vector"

namespace server {

/**
 * @brief Main class for the server. Handles listening on sockets and managing
 *  clients.
 */
class Server {
 public:
  /**
   * @brief Starts the server and listens indefinitely for connections.
   * @details This should run forever; if it ever returns, it is safe to assume
   *    it was because of an error.
   * @param port The port to listen on.
   */
  void Listen(uint16_t port);
private:

    ///Active Get and Put commands
    std::vector<int> ActiveCommands;
};

}  // namespace server

#endif  // PROJECT1_SERVER_H
