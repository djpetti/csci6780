#include "thread_pool.h"

#include <cassert>

namespace thread_pool {

ThreadPool::ThreadPool(uint32_t num_threads)
    : dispatcher_thread_(&ThreadPool::DispatcherThread, this),
      joiner_thread_(&ThreadPool::JoinerThread, this),
      max_pool_size_(num_threads) {}

ThreadPool::~ThreadPool() {
  {
    std::lock_guard<std::mutex> lock(mutex_);

    // Cancel all tasks.
    for (const auto& handle_and_task : handle_to_task_) {
      cancelled_tasks_.insert(handle_and_task.first);
    }

    // Indicate that we should stop the pool threads.
    should_close_ = true;
  }
  // Wake up the internal threads and force them to check should_close_.
  task_pending_.notify_one();
  task_done_.notify_one();

  // Wait for all threads to finish.
  dispatcher_thread_.join();
  joiner_thread_.join();

  // No need to hold the mutex for this since no other threads are running at
  // this point.
  for (auto& handle_and_thread : handle_to_thread_) {
    handle_and_thread.second.join();
  }
}

Task::Handle ThreadPool::AddTask(std::unique_ptr<Task>* task) {
  const auto kTaskHandle = (*task)->GetHandle();

  {
    std::lock_guard<std::mutex> lock(mutex_);

    // Add the task to the bookkeeping data structures.
    handle_to_task_.emplace((*task)->GetHandle(), std::move(*task));
    handle_to_status_[kTaskHandle] = Task::Status::RUNNING;

    // Add to the queue.
    dispatch_queue_.push(kTaskHandle);
  }

  // Notify dispatcher thread. We deliberately release the lock before doing
  // this to avoid waking up the waiting thread only to block again.
  task_pending_.notify_one();

  return kTaskHandle;
}

void ThreadPool::DispatcherThread() {
  while (true) {
    {
      std::unique_lock<std::mutex> lock(mutex_);

      // Check that we have a free thread.
      if (max_pool_size_ != 0 && pool_size_ >= max_pool_size_) {
        // Wait for a thread to become available.
        thread_available_.wait(lock,
                               [this] { return pool_size_ < max_pool_size_; });
      }

      // Wait for a new pending task.
      if (dispatch_queue_.empty() && !should_close_) {
        task_pending_.wait(
            lock, [this] { return !dispatch_queue_.empty() || should_close_; });
      }
      if (should_close_) {
        // We should exit now and avoid submitting new tasks.
        break;
      }

      ++pool_size_;

      // Create a thread for the task.
      const auto kTaskHandle = dispatch_queue_.back();
      dispatch_queue_.pop();

      const auto& task = handle_to_task_[kTaskHandle];
      std::thread task_thread(&ThreadPool::RunTask, this, task.get());
      handle_to_thread_[kTaskHandle] = std::move(task_thread);
    }
  }
}
void ThreadPool::JoinerThread() {
  while (true) {
    {
      std::unique_lock<std::mutex> lock(mutex_);

      // Wait for a new done task.
      if (joinable_queue_.empty() && !should_close_) {
        task_done_.wait(
            lock, [this] { return !joinable_queue_.empty() || should_close_; });
      }
      if (should_close_) {
        // We should exit now and avoid finalizing new tasks.
        break;
      }

      // Finalize the task.
      const auto kTaskHandle = joinable_queue_.back();
      joinable_queue_.pop();

      auto& thread = handle_to_thread_[kTaskHandle];
      thread.join();

      // Erase the task from the bookkeeping structures. Note that we maintain
      // a reference to the task status so that the status will be correctly
      // reported if it is checked later.
      handle_to_task_.erase(kTaskHandle);
      handle_to_thread_.erase(kTaskHandle);
      cancelled_tasks_.erase(kTaskHandle);

      // Indicate that we have a free thread.
      --pool_size_;
    }
    thread_available_.notify_one();
  }
}

bool ThreadPool::UpdateTaskStatus(Task::Handle handle, Task::Status status) {
  {
    std::lock_guard<std::mutex> lock(mutex_);

    // Check if the task should be cancelled.
    if (cancelled_tasks_.find(handle) != cancelled_tasks_.end()) {
      status = Task::Status::CANCELLED;
    }

    // Update the status record.
    handle_to_status_[handle] = status;
  }

  // Indicate whether we should exit or not.
  return status == Task::Status::RUNNING;
}

void ThreadPool::RunTask(Task* task) {
  // Perform one-time setup.
  Task::Status status = task->SetUp();

  // Run the task.
  while (UpdateTaskStatus(task->GetHandle(), status)) {
    status = task->RunAtomic();
  }

  // The task is finished. Perform cleanup.
  task->Cleanup();

  // Notify that the task is complete.
  {
    std::lock_guard<std::mutex> lock(mutex_);

    joinable_queue_.push(task->GetHandle());
  }
  task_done_.notify_one();
}

Task::Status ThreadPool::GetTaskStatus(const Task::Handle& task_handle) {
  std::lock_guard<std::mutex> lock(mutex_);

  const auto handle_and_status = handle_to_status_.find(task_handle);
  assert(handle_and_status != handle_to_status_.end() &&
         "Attempt to get status of nonexistent thread.");
  return handle_to_status_[task_handle];
}

void ThreadPool::CancelTask(const Task::Handle& task_handle) {
  std::lock_guard<std::mutex> lock(mutex_);
  cancelled_tasks_.insert(task_handle);
}

}  // namespace thread_pool