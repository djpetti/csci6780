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
   * @param config_file the config file of this name server
   */
  explicit NameserverTask(const std::filesystem::path config_file);

  Status SetUp() override;

  Status RunAtomic() override;

 private:
  /// Nameserver
  nameserver::Nameserver nameserver_;

  /// Command queue
  queue::Queue<std::pair<nameserver::NameserverCommand, std::string>> cmd_queue_;

};
}  // namespace nameserver::tasks
#endif  // PROJECT4_NAMESERVER_TASK_H
