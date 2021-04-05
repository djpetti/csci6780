#include "console_task.h"

#include <loguru.hpp>
#include <utility>

namespace participant_tasks {

ConsoleTask::ConsoleTask(std::string prompt) : prompt_(std::move(prompt)) {}

thread_pool::Task::Status ConsoleTask::SetUp() {
  std::cout << prompt_ << std::flush;
  return thread_pool::Task::Status::RUNNING;
}

thread_pool::Task::Status ConsoleTask::RunAtomic() {
  if (console_msg_buf_.empty()) {
    return thread_pool::Task::Status::RUNNING;
  }
  // Clear prompt line in expectation of incoming console statement
  ClearLine();
  for (const std::string& msg : console_msg_buf_) {
    // Make sure each oncoming console statement is printed
    std::cout << msg << std::endl;
  }
  // Clear console output buffer
  console_msg_buf_.erase(console_msg_buf_.begin(), console_msg_buf_.end());
  // Display prompt for end-user
  std::cout << prompt_ << std::flush;
  return thread_pool::Task::Status::RUNNING;
}

void ConsoleTask::SendConsole(const std::string& message) {
  console_msg_buf_.push_back(message);
}

void ConsoleTask::ClearLine() { std::cout << "\033[A\33[2K\r"; }

}  // namespace participant_tasks