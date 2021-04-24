/**
 * @file Nameserver Driver
 */
#ifndef PROJECT4_NAMESERVER_DRIVER_H
#define PROJECT4_NAMESERVER_DRIVER_H

#include "../../common/thread_pool/task.h"
#include "../../common/thread_pool/thread_pool.h"

#include "tasks/console_task.h"
#include "tasks/nameserver_task.h"

namespace nameserver {

class NameserverDriver {
 public:
  explicit NameserverDriver(std::string config_file);

  [[noreturn]] void Start();

 private:
  /// Pool and tasks
  thread_pool::ThreadPool pool_;
  std::shared_ptr<nameserver_tasks::ConsoleTask> console_task_;
  std::shared_ptr<nameserver_tasks::NameserverTask> nameserver_task_;

  /// Config file
  const std::string config_file_;
};

}  // namespace nameserver
#endif  // PROJECT4_NAMESERVER_DRIVER_H