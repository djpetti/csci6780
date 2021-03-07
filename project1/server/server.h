/**
 * @file Main class for the server.
 */

#ifndef PROJECT1_SERVER_H
#define PROJECT1_SERVER_H

#include <cstdint>
#include "vector"
#include "../thread_pool/thread_pool.h"
#include "server_tasks/command_ids.h"
#include "file_handler/file_access_manager.h"

namespace server {

/**
 * @brief Main class for the server. Handles listening on sockets and managing
 *  clients.
 */
class Server {
 public:
  /**
   * @brief Starts the FTP service
   * @details This should run forever; if it ever returns, it is safe to assume
   *    it was because of an error.
   * @param nPort The port # for normal commands.
   * @param tPort The port # for terminate commands.
   */
  void FtpService(uint16_t nPort, uint16_t tPort);
private:

    ///File access managers. @note To be given to nPortTask and tPort Task
    std::shared_ptr<file_handler::FileAccessManager> read_manager_;
    std::shared_ptr<file_handler::FileAccessManager> write_manager_;

};

}  // namespace server

#endif  // PROJECT1_SERVER_H
