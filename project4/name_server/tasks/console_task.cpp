#include "console_task.h"

#include <loguru.hpp>
#include <utility>

namespace nameserver::tasks {

ConsoleTask::ConsoleTask(const std::string& prompt) : prompt_(prompt) {}

thread_pool::Task::Status ConsoleTask::SetUp() {
  std::cout << prompt_ << std::flush;
  return thread_pool::Task::Status::RUNNING;
}

thread_pool::Task::Status ConsoleTask::RunAtomic() {
  std::string first_message;
  if (!console_message_queue_.PopTimed(kTimeout, &first_message)) {
    return Status::RUNNING;
  }
  // Clear prompt line in expectation of incoming console statement
  ClearLine();

  std::cout << first_message << std::endl;
  // Print any additional lines consecutively.
  while (!console_message_queue_.Empty()) {
    std::cout << console_message_queue_.Pop() << std::endl;
  }

  // Display prompt for end-user
  std::cout << prompt_ << std::flush;
  return thread_pool::Task::Status::RUNNING;
}

void ConsoleTask::SendConsole(const std::string& message) {
  console_message_queue_.Push(message);
}

void ConsoleTask::ClearLine() { std::cout << "\033[A\33[2K\r"; }

}  // namespace nameserver::tasks