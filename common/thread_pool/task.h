#ifndef PROJECT1_TASK_H
#define PROJECT1_TASK_H

#include <atomic>
#include <cstdint>

namespace thread_pool {

/**
 * @brief Represents a task that can be run in a thread pool.
 */
class Task {
 public:
  /// Represents an opaque identifier for a task.
  using Handle = uint32_t;

  /**
   * @brief Represents the status of a task.
   */
  enum class Status {
    /// Task is running normally.
    RUNNING,
    /// Task has completed normally.
    DONE,
    /// Task has failed.
    FAILED,
    /// Task was cancelled.
    CANCELLED,
  };

  Task();

  virtual ~Task() = default;

  /**
   * @brief Performs any necessary one-time setup when the task starts.
   * @return The updated status for this task.
   */
  virtual Status SetUp();

  /**
   * @brief Runs one iteration of the task main loop.
   * @note The task cannot be cancelled in the middle of this function. However,
   *    you should assume that cancellation is possible between calls.
   * @return The updated status for this task.
   */
  virtual Status RunAtomic() = 0;

  /**
   * @brief Will be called by the pool to perform any cleanup after the
   *    task status changes to either DONE or FAILED.
   */
  virtual void CleanUp();

  /**
   * @brief Gets a unique handle for this task.
   */
  [[nodiscard]] Handle GetHandle() const;

 private:
  /// Static counter we use for generating task IDs.
  static std::atomic<uint32_t> current_id_;
  /// The ID of this instance.
  uint32_t id_ = 0;
};

}  // namespace thread_pool

#endif  // PROJECT1_TASK_H
