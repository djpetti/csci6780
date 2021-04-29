/**
 * @file Bootstrap Driver
 */
#ifndef PROJECT4_BOOTSTRAP_DRIVER_H
#define PROJECT4_BOOTSTRAP_DRIVER_H

#include "bootstrap.h"
#include "tasks/console_task.h"
#include "tasks/nameserver_task.h"
#include "thread_pool/task.h"
#include "thread_pool/thread_pool.h"

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
  void Start();

 private:
  /**
   * @param config_loc The location of the configuration
   */
  void LoadConfig(const std::filesystem::path& config_loc);

  /// Whether this driver is supposed to be running
  bool running_ = true;

  /// Bootstrap
  std::shared_ptr<nameserver::Bootstrap> bootstrap_;

  /// Pool and tasks
  std::shared_ptr<thread_pool::ThreadPool> pool_;
  std::shared_ptr<nameserver::tasks::ConsoleTask> console_task_;
  std::shared_ptr<nameserver::tasks::NameserverTask> nameserver_task_;
};
}  // namespace nameserver
#endif  // PROJECT4_BOOTSTRAP_DRIVER_H
