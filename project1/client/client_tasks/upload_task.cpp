#include "upload_task.h"

namespace client_tasks {
UploadTask::UploadTask(int client_fd) : client_fd_(client_fd) {};
thread_pool::Task::Status UploadTask::RunAtomic() {
  if (send(client_fd_, outgoing_file_buf_.data(), outgoing_file_buf_.size(), 0) <
      0) {
    perror("Failed to send request");
    return thread_pool::Task::Status::FAILED;
  }
  return thread_pool::Task::Status::DONE;
}
void UploadTask::CleanUp() {
  outgoing_file_buf_.clear();
}
}
