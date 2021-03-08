#ifndef PROJECT1_UPLOAD_TASK_H
#define PROJECT1_UPLOAD_TASK_H

#include "../../thread_pool/task.h"
#include "../../wire_protocol/wire_protocol.h"
#include "ftp_messages.pb.h"

namespace client_tasks {
class UploadTask : public thread_pool::Task {
 public:
  UploadTask(int client_fd, std::vector<uint8_t> &outgoing_buf) {
    client_fd_ = client_fd;
    outgoing_file_buf_ = outgoing_buf;
  }
  Status RunAtomic() override;

  void CleanUp() override;

 protected:

  // client socket
  int client_fd_;

  // outgoing buffer that stores serialized data to be sent to the server
  std::vector<uint8_t> outgoing_file_buf_{};

};
}
#endif  // PROJECT1_UPLOAD_TASK_H
