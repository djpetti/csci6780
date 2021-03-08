#include "terminate_task.h"

namespace client_tasks {
thread_pool::Task::Status TerminateTask::SetUp() {
  socket_ = client_util::SetUpSocket( client_util::MakeAddress(port_), address_);
  return thread_pool::Task::Status::RUNNING;
}
thread_pool::Task::Status TerminateTask::RunAtomic() {
  wire_protocol::Serialize(terminate_req_, &outgoing_terminate_buf_);
  if (send(socket_, outgoing_terminate_buf_.data(),
           outgoing_terminate_buf_.size(), 0) < 0) {
    perror("Failed to send request");
    return thread_pool::Task::Status::FAILED;
  }
  outgoing_terminate_buf_.clear();
  return thread_pool::Task::Status::RUNNING;
}
void TerminateTask::CleanUp() {
  close(socket_);
}
}
