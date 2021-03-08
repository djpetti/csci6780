#ifndef PROJECT1_DOWNLOAD_TASK_H
#define PROJECT1_DOWNLOAD_TASK_H

#include "../../thread_pool/task.h"
#include "../../wire_protocol/wire_protocol.h"
#include "../client_util.h"

namespace client_tasks {
class DownloadTask : public thread_pool::Task {
 public:
  DownloadTask(const std::string& filename, std::vector<uint8_t> &incoming_buf,
               size_t buf_size, int client_fd,
               wire_protocol::MessageParser<google::protobuf::Message> &parser
               ) {
    filename_ = filename;
    incoming_file_buf_ = incoming_buf;
    buffer_size_ = buf_size;
    client_fd_ = client_fd;
    parser_ = parser;
  }

  Status RunAtomic() override;

  void CleanUp() override;

 protected:
  std::string filename_{};
  std::vector<uint8_t> incoming_file_buf_{};
  int client_fd_;
  size_t buffer_size_;
  wire_protocol::MessageParser<google::protobuf::Message> parser_;
};
}
#endif  // PROJECT1_DOWNLOAD_TASK_H
