#include "download_task.h"

namespace client_tasks {
DownloadTask::DownloadTask(std::string filename, size_t buf_size,
                           int client_fd) : filename_(filename), client_fd_(client_fd), buffer_size_(buf_size) {};
thread_pool::Task::Status DownloadTask::RunAtomic() {
  while(!parser_.HasCompleteMessage()) {
    incoming_file_buf_.resize(buffer_size_);
    const auto bytes_read =
        recv(client_fd_, incoming_file_buf_.data(), buffer_size_, 0);
    if (bytes_read < 1) {
      perror("Connection error.");
    }
    incoming_file_buf_.resize(bytes_read);
    parser_.AddNewData(incoming_file_buf_);
  }
  ftp_messages::FileContents contents;
  parser_.GetMessage(&contents);
  client_util::SaveIncomingFile(contents.contents(), filename_);
  return thread_pool::Task::Status::DONE;
}

void DownloadTask::CleanUp() {
  incoming_file_buf_.clear();
}
}
