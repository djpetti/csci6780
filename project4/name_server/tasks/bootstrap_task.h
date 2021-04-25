/**
 * @file Bootstrap Task
 */
#ifndef PROJECT4_BOOTSTRAP_TASK_H
#define PROJECT4_BOOTSTRAP_TASK_H

#include <iostream>
#include "thread_pool/task.h"
#include "../bootstrap.h"
#include "../commands.h"
#include "queue/queue.h"

namespace nameserver::tasks {

/**
 * @class The task that controls the contained bootstrap
 */
class BootstrapTask : public thread_pool::Task {
 public:

  /**
   * @brief Initializes the bootstrap task based on config file.
   * @param config_file the config file of this bootstrap
   */
  explicit BootstrapTask(const std::filesystem::path config_file);

  Status SetUp() override;

  Status RunAtomic() override;

 private:
  /// Bootstrap
  nameserver::Bootstrap bootstrap_;

  /// Command queue
  queue::Queue<std::pair<nameserver::BootstrapCommand, std::string>> cmd_queue_;

};
}  // namespace nameserver::tasks
#endif  // PROJECT4_BOOTSTRAP_TASK_H
