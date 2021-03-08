#ifndef PROJECT1_TERMINATE_TASK_H
#define PROJECT1_TERMINATE_TASK_H

#include "../../thread_pool/task.h"
#include "../../wire_protocol/wire_protocol.h"
#include "ftp_messages.pb.h"
#include "../client_util.h"

#include <sys/socket.h>
#include <utility>
#include <unistd.h>

namespace client_tasks {
class TerminateTask : public thread_pool::Task {
 public:
  TerminateTask(const std::string &address, uint16_t port,
                ftp_messages::TerminateRequest terminate_req) {
    address_ = address;
    port_ = port;
    terminate_req_ = std::move(terminate_req);
  }

  thread_pool::Task::Status SetUp() override;

  thread_pool::Task::Status RunAtomic() override;

  void CleanUp() override;

 protected:

  // outgoing buffer that stores serialized data to be sent to the server
  std::vector<uint8_t> outgoing_terminate_buf_{};

  // address of the server
  std::string address_{};

  // termination port of the server
  uint16_t port_;

  // the encapsulated termination request
  ftp_messages::TerminateRequest terminate_req_;

  // socket used to send the termination request
  int socket_{};

};
}
#endif  // PROJECT1_TERMINATE_TASK_H
