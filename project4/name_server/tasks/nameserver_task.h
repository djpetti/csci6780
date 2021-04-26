/**
 * @file Name Server Task
 */
#ifndef PROJECT4_NAMESERVER_TASK_H
#define PROJECT4_NAMESERVER_TASK_H

#include <iostream>

#include "../commands.h"
#include "../nameserver.h"
#include "queue/queue.h"
#include "thread_pool/task.h"

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
};
}  // namespace nameserver::tasks
#endif  // PROJECT4_NAMESERVER_TASK_H
