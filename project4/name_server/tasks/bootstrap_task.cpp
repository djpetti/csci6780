#include "bootstrap_task.h"

namespace nameserver_tasks {

/// TODO Instantiate bootstrap
BootstrapTask::BootstrapTask(const std::string &config_file)
    : bootstrap_(config_file) {}

/// TODO Set up socket listening
thread_pool::Task::Status BootstrapTask::SetUp() {
  return Task::SetUp();
}

/// TODO Listen to all requests. Pass them accordingly to HandleResponse
/// If there is a command in cmd_queue_, parse and execute.
thread_pool::Task::Status BootstrapTask::RunAtomic() {
  return thread_pool::Task::Status::RUNNING;
}

}  // namespace nameserver_tasks
