#ifndef CSCI6780_QUEUE_H
#define CSCI6780_QUEUE_H

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <queue>

#include <loguru.hpp>

namespace queue {

/**
 * @brief Implements a thread-safe blocking queue.
 * @tparam T The type of object this queue will store.
 */
template <class T>
class Queue {
 public:
  /**
   * @param max_length Maximum number of elements to allow in the queue. By
   *    default, there is no limit.
   */
  explicit Queue(uint32_t max_length = 0) : max_length_(max_length){};

  /**
   * @brief Pushes a new element onto the queue.
   * @param element The element to push.
   */
  void Push(const T& element) {
    {
      std::unique_lock<std::mutex> lock(mutex_);

      // Check that the queue is not full.
      if (max_length_ != 0 && queue_.size() >= max_length_) {
        // Wait for the queue not to be full.
        queue_not_full_.wait(lock,
                             [this] { return queue_.size() < max_length_; });
      }

      // Push onto the queue.
      queue_.push(element);
    }

    // Notify that the queue is no longer empty.
    queue_not_empty_.notify_one();
  }

  /**
   * @brief Pops an element from the queue.
   * @return The element from the queue.
   */
  T Pop() {
    T element;
    while (!PopTimed(std::chrono::hours(1), &element))
      ;

    return element;
  }

  /**
   * @brief Same as `Pop()`, but blocks for a maximum amount of time before
   *    failing.
   * @tparam Rep The underlying numeric type for the duration.
   * @tparam Period The underlying period for the duration.
   * @param timeout The timeout.
   * @param element[out] The output element will be written here.
   * @return True if it successfully popped from the queue, false if the
   *    operation timed out.
   */
  template <class Rep, class Period>
  bool PopTimed(const std::chrono::duration<Rep, Period>& timeout, T* element) {
    std::unique_lock<std::mutex> lock(mutex_);

    // Check that the queue is not empty.
    if (queue_.empty()) {
      // Wait for the queue not to be empty.
      if (!queue_not_empty_.wait_for<Rep, Period>(
          lock, timeout, [this] { return !queue_.empty(); })) {
        // Timeout expired.
        LOG_S(1) << "Queue pop timeout expired.";
        return false;
      }
    }

    // Pop from the queue.
    *element = queue_.front();
    queue_.pop();

    // Notify that the queue is no longer full.
    queue_not_full_.notify_one();

    return true;
  }

  /**
   * @return True if the queue is empty.
   */
  bool Empty() {
    std::lock_guard<std::mutex> lock(mutex_);

    return queue_.empty();
  }

 private:
  /// The maximum number of elements allowed in the queue.
  uint32_t max_length_;
  /// Underlying non-thread-safe queue.
  std::queue<T> queue_;

  /// Mutex to use for protecting queue access.
  std::mutex mutex_{};
  /// Condition variable indicating that the queue is not empty.
  std::condition_variable queue_not_empty_{};
  /// Condition variable indicating that the queue is not full.
  std::condition_variable queue_not_full_{};
};

}  // namespace queue

#endif  // CSCI6780_QUEUE_H
