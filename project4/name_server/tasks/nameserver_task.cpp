#include "nameserver_task.h"

namespace nameserver::tasks {

NameserverTask::NameserverTask(
    std::shared_ptr<nameserver::Nameserver> nameserver)
    : nameserver_(nameserver) {}

thread_pool::Task::Status NameserverTask::RunAtomic() {
  nameserver_->ReceiveAndHandle();
  return thread_pool::Task::Status::RUNNING;
}

}  // namespace nameserver::tasks
