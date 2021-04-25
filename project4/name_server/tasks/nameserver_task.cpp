#include "nameserver_task.h"

namespace nameserver::tasks {

/// TODO Instantiate name server
NameserverTask::NameserverTask(const std::filesystem::path config_file)
    : nameserver_(config_file) {}

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
