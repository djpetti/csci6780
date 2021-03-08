#include "download_task.h"

namespace client_tasks {
thread_pool::Task::Status DownloadTask::RunAtomic() {
  while(!parser_.HasCompleteMessage()) {
    incoming_file_buf_.resize(buffer_size_);
    recv(client_fd_, incoming_file_buf_.data(), buffer_size_, 0);
    incoming_file_buf_.resize(buffer_size_);
    parser_.AddNewData(incoming_file_buf_);
  }
  client_util::SaveIncomingFile(parser_, filename_);
  return thread_pool::Task::Status::DONE;
}

void DownloadTask::CleanUp() {
  incoming_file_buf_.clear();
}
}
