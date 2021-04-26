/**
 * @file Bootstrap Task
 */
#ifndef PROJECT4_BOOTSTRAP_TASK_H
#define PROJECT4_BOOTSTRAP_TASK_H

#include <iostream>

#include "../bootstrap.h"
#include "../commands.h"
#include "queue/queue.h"
#include "thread_pool/task.h"

namespace nameserver::tasks {

/**
 * @class The task that controls the contained bootstrap
 */
class BootstrapTask : public thread_pool::Task {
 public:
  /**
   * @brief Initializes the bootstrap task based on config file.
   * @param nameserver the bootstrap associated with this task
   */
  explicit BootstrapTask(std::shared_ptr<nameserver::Bootstrap> bootstrap);

  Status SetUp() override;

  Status RunAtomic() override;

 private:
  /// Bootstrap
  std::shared_ptr<nameserver::Bootstrap> bootstrap_;
};
}  // namespace nameserver::tasks
#endif  // PROJECT4_BOOTSTRAP_TASK_H
