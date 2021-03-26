/**
 * @file Task to upload a file to the server asynchronously
 */

#ifndef PROJECT1_UPLOAD_TASK_H
#define PROJECT1_UPLOAD_TASK_H

#include "thread_pool/task.h"
#include "wire_protocol/wire_protocol.h"
#include "chunked_files/chunked_file_sender.h"
#include "ftp_messages.pb.h"

namespace client_tasks {

class UploadTask : public thread_pool::Task {
 public:
  /**
   * @param client_fd The socket used to upload the FileContents
   * @param file_data The buffer containing the file data to upload.
   */
  explicit UploadTask(int client_fd, const std::vector<uint8_t>& file_data);

  Status RunAtomic() override;

 protected:
  /// client socket
  int client_fd_;

  /// Chunked file sender
  chunked_files::ChunkedFileSender sender_;
};
}  // namespace client_tasks
#endif  // PROJECT1_UPLOAD_TASK_H
