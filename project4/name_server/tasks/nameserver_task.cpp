#include "nameserver_task.h"

#include <utility>

namespace nameserver::tasks {

/// TODO Instantiate name server
NameserverTask::NameserverTask(std::shared_ptr<nameserver::Nameserver> nameserver)
    : nameserver_(nameserver) {}

/// TODO Set up socket listening
thread_pool::Task::Status NameserverTask::SetUp() {
  return Task::SetUp();
}

/// TODO Listen to all requests. Pass them accordingly to HandleResponse
/// If there is a command in cmd_queue_, parse and execute.
thread_pool::Task::Status NameserverTask::RunAtomic() {
  return thread_pool::Task::Status::RUNNING;
}

}  // namespace nameserver::tasks
