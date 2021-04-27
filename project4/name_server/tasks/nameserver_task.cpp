#include "nameserver_task.h"

#include <utility>

namespace nameserver::tasks {

NameserverTask::NameserverTask(
    std::shared_ptr<nameserver::Nameserver> nameserver)
    : nameserver_(nameserver) {}

thread_pool::Task::Status NameserverTask::RunAtomic() {
  message_passing::Endpoint endpoint;
  consistent_hash_msgs::NameServerMessage ns_msg;
  if (nameserver_->server_->Receive(kTimeout, &ns_msg, &endpoint)) {
    nameserver_->HandleRequest(ns_msg, endpoint);
  }
  return thread_pool::Task::Status::RUNNING;
}

}  // namespace nameserver::tasks
