/**
 * @file Task to upload a file to the server asynchronously
 */

#ifndef PROJECT1_UPLOAD_TASK_H
#define PROJECT1_UPLOAD_TASK_H

#include "../../thread_pool/task.h"
#include "../../wire_protocol/wire_protocol.h"
#include "ftp_messages.pb.h"

namespace client_tasks {
class UploadTask : public thread_pool::Task {
 public:
  /**
   * @param client_fd The socket used to upload the FileContents
   */
  explicit UploadTask(int client_fd);

  Status RunAtomic() override;

  void CleanUp() override;

 protected:

  /// client socket
  int client_fd_;

  /// outgoing buffer that stores serialized data to be sent to the server
  std::vector<uint8_t> outgoing_file_buf_{};

};
}
#endif  // PROJECT1_UPLOAD_TASK_H
