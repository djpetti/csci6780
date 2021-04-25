/**
 * @file Name Server Task
 */
#ifndef PROJECT4_NAMESERVER_TASK_H
#define PROJECT4_NAMESERVER_TASK_H

#include <iostream>
#include "thread_pool/task.h"
#include "../nameserver.h"
#include "../commands.h"
#include "queue/queue.h"

namespace nameserver::tasks {

/**
 * @class The task that controls the contained nameserver
 */
class NameserverTask : public thread_pool::Task {
 public:

  /**
   * @brief Initializes the nameserver task based on config file.
   * @param nameserver the nameserver associated with this task
   */
  explicit NameserverTask(std::shared_ptr<nameserver::Nameserver> nameserver);

  Status SetUp() override;

  Status RunAtomic() override;

 private:
  /// Nameserver
  std::shared_ptr<nameserver::Nameserver> nameserver_;

  /// Command queue
  queue::Queue<std::pair<nameserver::NameserverCommand, std::string>> cmd_queue_;

};
}  // namespace nameserver::tasks
#endif  // PROJECT4_NAMESERVER_TASK_H
