/**
 * @file Name Server Task
 */
#ifndef PROJECT4_NAMESERVER_TASK_H
#define PROJECT4_NAMESERVER_TASK_H

#include <iostream>
#include "thread_pool/task.h"
#include "../nameserver.h"
#include "../commands.h"
#include "../../../common/queue/queue.h"

namespace nameserver_tasks {

class NameserverTask : public thread_pool::Task {
 public:
  explicit NameserverTask(const std::string& config_file);

  Status SetUp() override;

  Status RunAtomic() override;

 private:
  /// Nameserver
  nameserver::Nameserver nameserver_;

  /// Command queue
  queue::Queue<std::pair<nameserver::NameserverCommand, std::string>> cmd_queue_;

};
}  // namespace nameserver_tasks
#endif  // PROJECT4_NAMESERVER_TASK_H
