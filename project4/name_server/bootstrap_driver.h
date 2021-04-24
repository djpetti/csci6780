/**
 * @file Bootstrap Driver
 */
#ifndef PROJECT4_BOOTSTRAP_DRIVER_H
#define PROJECT4_BOOTSTRAP_DRIVER_H

#include "../../common/thread_pool/task.h"
#include "../../common/thread_pool/thread_pool.h"

#include "tasks/console_task.h"
#include "tasks/bootstrap_task.h"

namespace nameserver {

class BootstrapDriver {
 public:
  explicit BootstrapDriver(const std::string& config_file);

  [[noreturn]] void Start();

 private:
  /// Pool and tasks
  thread_pool::ThreadPool pool_;
  std::shared_ptr<nameserver_tasks::ConsoleTask> console_task_;
  std::shared_ptr<nameserver_tasks::BootstrapTask> bootstrap_task_;
};
}  // namespace nameserver
#endif  // PROJECT4_BOOTSTRAP_DRIVER_H
