#ifndef PROJECT1_THREAD_POOL_H
#define PROJECT1_THREAD_POOL_H

#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>
#include <unordered_set>

#include "thread_pool_interface.h"

namespace thread_pool {

/**
 * @brief Standard thread pool implementation.
 */
class ThreadPool : public IThreadPool {
 public:
  /**
   * @param num_threads The number of threads in the pool. By default, it
   *    will create threads on-demand.
   */
  explicit ThreadPool(uint32_t num_threads = 0);

  ~ThreadPool() override;

  void AddTask(const std::shared_ptr<Task> &task) final;
  Task::Status GetTaskStatus(const std::shared_ptr<Task> &task) final;
  void CancelTask(const std::shared_ptr<Task> &task_handle) final;
  void WaitForCompletion() final;
  uint32_t NumThreads() final;

 private:
  /**
   * @brief Thread that's in charge of dispatching new tasks to the pool.
   */
  void DispatcherThread();

  /**
   * @brief Thread that's in charge of finalizing completed tasks.
   */
  void JoinerThread();

  /**
   * @brief Entry point for new threads running tasks.
   * @param task The task to run.
   */
  void RunTask(Task *task);

  /**
   * Performs bookkeeping updates for the task status.
   * @param handle The handle of the task.
   * @param status The current status of that task.
   * @return Whether we should keep running the task.
   */
  bool UpdateTaskStatus(const Task::Handle &handle, Task::Status status);

  /// Maps task handles to task objects.
  std::unordered_map<Task::Handle, std::shared_ptr<Task>> handle_to_task_{};
  /// Maps task handles to statuses.
  std::unordered_map<Task::Handle, Task::Status> handle_to_status_{};
  /// Maps task handles to threads.
  std::unordered_map<Task::Handle, std::thread> handle_to_thread_{};
  /// Handles of tasks that should be cancelled.
  std::unordered_set<Task::Handle> cancelled_tasks_{};

  /**
   * @brief Indicates that we have a pending task that's ready to run. Also used
   *    to indicate that the pool is being closed.
   */
  std::condition_variable task_pending_;
  /**
   * @brief Indicates that we have a completed task that needs to be finalized.
   *    Also used to indicate that the pool is being closed.
   */
  std::condition_variable task_done_;
  /// Indicates that we have a new idle thread available.
  std::condition_variable thread_available_;

  /// Internal queue for sending tasks to the dispatcher thread.
  std::queue<Task::Handle> dispatch_queue_;
  /// Internal queue for sending tasks to the joiner thread.
  std::queue<Task::Handle> joinable_queue_;
  /// If set, indicates that we should close the thread pool.
  bool should_close_ = false;

  /// Mutex to use for synchronization among all threads.
  std::mutex mutex_;

  /// Thread for dispatching new tasks.
  std::thread dispatcher_thread_;
  /// Thread for finalizing completed tasks.
  std::thread joiner_thread_;

  /**
   * @brief Maximum number of threads in the pool, or zero if there is no limit.
   */
  uint32_t max_pool_size_;
  /// Current number of threads in the pool.
  uint32_t pool_size_ = 0;
  /**
   * @brief Total number of completed tasks.
   * @note It is okay if this wraps; it is just used to check the completion
   *   of a new task by `WaitForCompletion`.
   */
   uint32_t num_completed_tasks_ = 0;
};

}  // namespace thread_pool

#endif  // PROJECT1_THREAD_POOL_H