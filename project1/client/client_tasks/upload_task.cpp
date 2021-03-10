#include "upload_task.h"

#include <utility>
#include <thread>

namespace client_tasks {

UploadTask::UploadTask(int client_fd,
                       std::vector<uint8_t> file_data)
    : client_fd_(client_fd), outgoing_file_buf_(std::move(file_data)) {}

thread_pool::Task::Status UploadTask::RunAtomic() {

  if (send(client_fd_, outgoing_file_buf_.data(), outgoing_file_buf_.size(),
           0) < 0) {
    perror("Failed to send request");
    return thread_pool::Task::Status::FAILED;
  }
  return thread_pool::Task::Status::DONE;
}
void UploadTask::CleanUp() { outgoing_file_buf_.clear(); }
}  // namespace client_tasks
