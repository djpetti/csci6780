#include "console_task.h"

#include <loguru.hpp>
#include <utility>

namespace participant_tasks {

ConsoleTask::ConsoleTask(const std::string& prompt) : prompt_(prompt) {}

thread_pool::Task::Status ConsoleTask::SetUp() {
  std::cout << prompt_ << std::flush;
  return thread_pool::Task::Status::RUNNING;
}

thread_pool::Task::Status ConsoleTask::RunAtomic() {
  if (console_msg_buf_.Empty()) {
    return thread_pool::Task::Status::RUNNING;
  }
  // Clear prompt line in expectation of incoming console statement
  ClearLine();
  while (!console_msg_buf_.Empty()) {
    // Make sure each oncoming console statement is printed
    std::cout << console_msg_buf_.Pop() << std::endl;
  }
  // Display prompt for end-user
  std::cout << prompt_ << std::flush;
  return thread_pool::Task::Status::RUNNING;
}

void ConsoleTask::SendConsole(const std::string& message) {
  console_msg_buf_.Push(message);
}

void ConsoleTask::ClearLine() { std::cout << "\033[A\33[2K\r"; }

}  // namespace participant_tasks