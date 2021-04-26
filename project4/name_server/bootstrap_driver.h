/**
 * @file Bootstrap Driver
 */
#ifndef PROJECT4_BOOTSTRAP_DRIVER_H
#define PROJECT4_BOOTSTRAP_DRIVER_H

#include "thread_pool/task.h"
#include "thread_pool/thread_pool.h"
#include "tasks/console_task.h"
#include "tasks/bootstrap_task.h"

namespace nameserver {

/**
 * @class The driver called by main to handle the Bootstrap user interface
 */
class BootstrapDriver {
 public:
  /**
   * @param config_file The configuration file location
   */
  explicit BootstrapDriver(std::filesystem::path config_file);

  /**
   * Start the user input loop
   */
  [[noreturn]] void Start();

 private:
  /// Bootstrap
  std::shared_ptr<nameserver::Bootstrap> bootstrap_;

  /// Pool and tasks
  std::shared_ptr<thread_pool::ThreadPool> pool_;
  std::shared_ptr<nameserver::tasks::ConsoleTask> console_task_;
  std::shared_ptr<nameserver::tasks::BootstrapTask> bootstrap_task_;
};
}  // namespace nameserver
#endif  // PROJECT4_BOOTSTRAP_DRIVER_H
