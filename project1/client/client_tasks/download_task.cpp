#include "download_task.h"

#include <cerrno>
#include <loguru.hpp>
#include <utility>

namespace client_tasks {

DownloadTask::DownloadTask(std::string filename, int client_fd)
    : filename_(std::move(filename)),
      client_fd_(client_fd),
      receiver_(client_fd_){};

thread_pool::Task::Status DownloadTask::RunAtomic() {
  if (!receiver_.HasCompleteFile()) {
    const auto bytes_read = receiver_.ReceiveNextChunk();
    if (bytes_read < 0) {
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
        // This is merely a timeout, and we should just spin again.
        return thread_pool::Task::Status::RUNNING;
      }

      LOG_S(ERROR) << "Socket error: " << strerror(errno);
      return thread_pool::Task::Status::FAILED;
    } else if (bytes_read == 0) {
      // Orderly shutdown from server.
      return thread_pool::Task::Status::DONE;
    }

    return thread_pool::Task::Status::RUNNING;
  } else {
    // Parse the complete message.
    std::vector<uint8_t> contents;
    receiver_.GetFileContents(&contents);
    client_util::SaveIncomingFile({contents.begin(), contents.end()},
                                  filename_);

    return thread_pool::Task::Status::DONE;
  }
}

}  // namespace client_tasks
