#include "bootstrap_task.h"

#include <utility>

#include "message_passing/server.h"

namespace nameserver::tasks {

BootstrapTask::BootstrapTask(std::shared_ptr<nameserver::Bootstrap> bootstrap)
    : bootstrap_(bootstrap) {}

thread_pool::Task::Status BootstrapTask::RunAtomic() {
  message_passing::Endpoint endpoint;
  consistent_hash_msgs::BootstrapMessage bs_msg;
  if (bootstrap_->server_->Receive(kTimeout, &bs_msg, &endpoint)) {
    bootstrap_->HandleRequest(bs_msg, endpoint);
  }
  return thread_pool::Task::Status::RUNNING;
}

}  // namespace nameserver::tasks
