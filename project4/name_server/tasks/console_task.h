/**
 * @file Participant Console Task
 */
#ifndef PROJECT4_CONSOLE_TASK_H
#define PROJECT4_CONSOLE_TASK_H

#include <unistd.h>

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "queue/queue.h"
#include "thread_pool/task.h"

namespace nameserver::tasks {

/**
 * @class The coordinator of all nameserver console output
 */
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
  queue::Queue<std::string> console_message_queue_{};
  std::string prompt_;

  /// Timeout used for PopTimout()
  constexpr static const auto kTimeout = std::chrono::milliseconds(100);
};
}  // namespace nameserver::tasks

#endif  // PROJECT4_CONSOLE_TASK_H
