#include "bootstrap_task.h"

#include <utility>

namespace nameserver::tasks {

/// TODO Instantiate bootstrap
BootstrapTask::BootstrapTask(std::shared_ptr<nameserver::Bootstrap> bootstrap)
    : bootstrap_(bootstrap) {}

/// TODO Set up socket listening
thread_pool::Task::Status BootstrapTask::SetUp() {
  return Task::SetUp();
}

/// TODO Listen to all requests. Pass them accordingly to HandleResponse
/// If there is a command in cmd_queue_, parse and execute.
thread_pool::Task::Status BootstrapTask::RunAtomic() {
  return thread_pool::Task::Status::RUNNING;
}

}  // namespace nameserver::tasks
