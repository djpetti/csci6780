/**
 * @file Task for downloading a FileContents message asynchronously
 */

#ifndef PROJECT1_DOWNLOAD_TASK_H
#define PROJECT1_DOWNLOAD_TASK_H

#include "../../thread_pool/task.h"
#include "../client_util.h"
#include "../../chunked_files/chunked_file_receiver.h"

namespace client_tasks {
class DownloadTask : public thread_pool::Task {
 public:
  /**
   * @param filename the name of the file to be saved
   * @param client_fd the socket to retrieve the FileContents message
   */
  DownloadTask(std::string filename, int client_fd);

  Status RunAtomic() override;

  void CleanUp() override;

 protected:

  /// the name of the file to be retrieved
  std::string filename_{};

  /// client socket
  int client_fd_;

  /// Receiver for chunked files.
  chunked_files::ChunkedFileReceiver receiver_;
};
}
#endif  // PROJECT1_DOWNLOAD_TASK_H
