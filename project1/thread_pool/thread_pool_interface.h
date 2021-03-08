#ifndef PROJECT1_THREAD_POOL_INTERFACE_H
#define PROJECT1_THREAD_POOL_INTERFACE_H

#include <cstdint>
#include <memory>

#include "task.h"

namespace thread_pool {

/**
 * @brief Standardized interface for thread pools.
 */
class IThreadPool {
 public:
  virtual ~IThreadPool() = default;

  /**
   * @brief Adds a new task to the pool.
   * @param task The task to add.
   */
  virtual void AddTask(const std::shared_ptr<Task>& task) = 0;

  /**
   * @brief Gets the current status of task.
   * @param task The task..
   * @return The status of the task.
   */
  virtual Task::Status GetTaskStatus(const std::shared_ptr<Task>& task) = 0;

  /**
   * @brief Cancels a running task.
   * @note Note that this function is asynchronous; The task may possibly still
   *    be running when it returns. To verify that the task has actually
   *    exited, use `GetTaskStatus`.
   * @param task The task to cancel.
   */
  virtual void CancelTask(const std::shared_ptr<Task>& task) = 0;

  /**
   * @brief Blocks until at least one task completes. If no tasks are running,
   *    it returns immediately.
   */
  virtual void WaitForCompletion() = 0;

  /**
   * @return Number of threads that are currently running.
   */
  virtual uint32_t NumThreads() = 0;
};

}  // namespace thread_pool

#endif  // PROJECT1_THREAD_POOL_INTERFACE_H
