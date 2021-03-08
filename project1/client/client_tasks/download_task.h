/**
 * @file Task for downloading a FileContents message asynchronously
 */

#ifndef PROJECT1_DOWNLOAD_TASK_H
#define PROJECT1_DOWNLOAD_TASK_H

#include "../../thread_pool/task.h"
#include "../../wire_protocol/wire_protocol.h"
#include "../client_util.h"

namespace client_tasks {
class DownloadTask : public thread_pool::Task {
 public:
  /**
   * @param filename the name of the file to be saved
   * @param buf_size the size of the buffer defined by client
   * @param client_fd the socket to retreive the FileContents message
   */
  DownloadTask(const std::string& filename, size_t buf_size, int client_fd);

  Status RunAtomic() override;

  void CleanUp() override;

 protected:

  /// the name of the file to be retreived
  std::string filename_{};

  /// incoming buffer that stores serialized data to be received from the server
  std::vector<uint8_t> incoming_file_buf_{};

  /// client socket
  int client_fd_;

  /// buffer size, provided by client
  size_t buffer_size_;

  /// parser used to parse incoming FileContents messages
  wire_protocol::MessageParser<ftp_messages::FileContents> parser_;
};
}
#endif  // PROJECT1_DOWNLOAD_TASK_H
