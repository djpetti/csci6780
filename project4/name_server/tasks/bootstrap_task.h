/**
 * @file Bootstrap Task
 */
#ifndef PROJECT4_BOOTSTRAP_TASK_H
#define PROJECT4_BOOTSTRAP_TASK_H

#include <iostream>
#include "thread_pool/task.h"
#include "../bootstrap.h"
#include "../commands.h"
#include "../../../common/queue/queue.h"

namespace nameserver_tasks {

class BootstrapTask : public thread_pool::Task {
 public:
  explicit BootstrapTask(const std::string& config_file);

  Status SetUp() override;

  Status RunAtomic() override;

 private:
  /// Bootstrap
  nameserver::Bootstrap bootstrap_;

  /// Command queue
  queue::Queue<std::pair<nameserver::BootstrapCommand, std::string>> cmd_queue_;

};
}
#endif  // PROJECT4_BOOTSTRAP_TASK_H
