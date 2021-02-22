#ifndef PROJECT1_THREAD_POOL_INTERFACE_H
#define PROJECT1_THREAD_POOL_INTERFACE_H

#include <memory>

#include "task.h"

namespace thread_pool {

/**
 * @brief Standardized interface for thread pools.
 */
class IThreadPool {
 public:
  /**
   * @brief Adds a new task to the pool.
   * @param task The task to add. The pool will take ownership of it.
   * @return A handle to the task in the pool.
   */
  virtual Task::Handle AddTask(const std::unique_ptr<Task>& task) = 0;

  /**
   * @brief Gets the current status of task.
   * @param task_handle The handle of the task.
   * @return The status of the task.
   */
  virtual Task::Status GetTaskStatus(const Task::Handle& task_handle) = 0;

  /**
   * @brief Cancels a running task.
   * @note Note that this function is asynchronous; The task may possibly still
   *    be running when it returns. To verify that the task has actually
   *    exited, use `GetTaskStatus`.
   * @param task_handle The task to cancel.
   */
  virtual void CancelTask(const Task::Handle& task_handle) = 0;
};

}  // namespace thread_pool

#endif  // PROJECT1_THREAD_POOL_INTERFACE_H
