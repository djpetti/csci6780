/**
 * @file Participant Console Task
 */
#ifndef PROJECT3_CONSOLE_TASK_H
#define PROJECT3_CONSOLE_TASK_H

#include <unistd.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "thread_pool/task.h"

namespace participant_tasks {
class ConsoleTask : public thread_pool::Task {
 public:
  /**
   * @param prompt the prompt to be displayed for this console
   */
  explicit ConsoleTask(const std::string& prompt);

  Status SetUp() override;

  Status RunAtomic() override;

  /**
   * @param message the message to be sent to this console
   */
  void SendConsole(const std::string& message);

 private:
  /**
   * clear the line of the console that the cursor is currently on
   */
  void ClearLine();

  std::vector<std::string> console_msg_buf_{};
  std::string prompt_;
};
}  // namespace participant_tasks

#endif  // PROJECT3_CONSOLE_TASK_H
