#include "upload_task.h"

#include <cerrno>
#include <loguru.hpp>


namespace client_tasks {

UploadTask::UploadTask(int client_fd, const std::vector<uint8_t>& file_data)
    : client_fd_(client_fd), sender_(client_fd_) {
  sender_.SetFileContents(file_data);
}

thread_pool::Task::Status UploadTask::RunAtomic() {
  if (sender_.SentCompleteFile()) {
    return thread_pool::Task::Status::DONE;
  }

  const int kSendResult = sender_.SendNextChunk();

  if (kSendResult < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // Time out. Give it another try.
      return thread_pool::Task::Status::RUNNING;
    }

    LOG_S(ERROR) << "Socket error: " << strerror(errno);
    return thread_pool::Task::Status::FAILED;

  } else if (kSendResult == 0) {
    // Orderly shutdown from server. In this case, we assume that the server
    // side was terminated.
    LOG_S(INFO) << "Server disconnected, assuming termination.";
    return thread_pool::Task::Status::DONE;
  }

  return thread_pool::Task::Status::RUNNING;
}

}  // namespace client_tasks
