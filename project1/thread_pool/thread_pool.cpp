#include "thread_pool.h"

#include <loguru.hpp>

namespace thread_pool {

ThreadPool::ThreadPool(uint32_t num_threads)
    : dispatcher_thread_(&ThreadPool::DispatcherThread, this),
      joiner_thread_(&ThreadPool::JoinerThread, this),
      max_pool_size_(num_threads) {}

ThreadPool::~ThreadPool() {
  LOG_S(INFO) << "Closing thread pool.";

  {
    std::lock_guard<std::mutex> lock(mutex_);

    // Cancel all tasks.
    for (const auto& handle_and_task : handle_to_task_) {
      cancelled_tasks_.insert(handle_and_task.first);
    }

    // Indicate that we should stop the pool threads.
    should_close_ = true;
  }
  LOG_S(1) << "Joining internal threads...";
  // Wake up the internal threads and force them to check should_close_.
  task_pending_.notify_one();
  task_done_.notify_all();
  thread_available_.notify_one();

  // Wait for all threads to finish.
  dispatcher_thread_.join();
  joiner_thread_.join();

  LOG_S(1) << "Joining task threads...";
  // No need to hold the mutex for this since no other threads are running at
  // this point.
  for (auto& handle_and_thread : handle_to_thread_) {
    handle_and_thread.second.join();
  }
}

void ThreadPool::AddTask(const std::shared_ptr<Task>& task) {
  const auto kTaskHandle = task->GetHandle();
  LOG_S(INFO) << "Adding a new task with handle " << kTaskHandle << ".";

  {
    std::lock_guard<std::mutex> lock(mutex_);

    // Add the task to the bookkeeping data structures.
    handle_to_task_[kTaskHandle] = task;
    handle_to_status_[kTaskHandle] = Task::Status::RUNNING;

    // Add to the queue.
    dispatch_queue_.push(kTaskHandle);
  }

  // Notify dispatcher thread. We deliberately release the lock before doing
  // this to avoid waking up the waiting thread only to block again.
  task_pending_.notify_one();
}

void ThreadPool::DispatcherThread() {
  while (true) {
    {
      std::unique_lock<std::mutex> lock(mutex_);

      // Check that we have a free thread.
      if (max_pool_size_ != 0 && pool_size_ >= max_pool_size_ &&
          !should_close_) {
        // Wait for a thread to become available.
        LOG_S(1) << "Waiting for a free thread...";
        thread_available_.wait(lock, [this] {
          return pool_size_ < max_pool_size_ || should_close_;
        });
      }

      // Wait for a new pending task.
      if (dispatch_queue_.empty() && !should_close_) {
        LOG_S(1) << "Waiting for a new task...";
        task_pending_.wait(
            lock, [this] { return !dispatch_queue_.empty() || should_close_; });
      }
      if (should_close_) {
        // We should exit now and avoid submitting new tasks.
        LOG_S(INFO) << "Exiting dispatcher thread.";
        break;
      }

      ++pool_size_;
      LOG_S(2) << "Pool size is now " << pool_size_ << ".";

      // Create a thread for the task.
      const auto kTaskHandle = dispatch_queue_.front();
      dispatch_queue_.pop();
      LOG_S(1) << "Got a new task: " << kTaskHandle << ".";

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
        LOG_S(1) << "Waiting for a task to finish...";
        task_done_.wait(
            lock, [this] { return !joinable_queue_.empty() || should_close_; });
      }
      if (should_close_) {
        // We should exit now and avoid finalizing new tasks.
        LOG_S(INFO) << "Exiting joiner thread.";
        break;
      }

      // Finalize the task.
      const auto kTaskHandle = joinable_queue_.front();
      joinable_queue_.pop();
      LOG_S(1) << "Got a finished task: " << kTaskHandle << ".";

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
      LOG_S(2) << "Pool size is now " << pool_size_ << ".";
    }
    thread_available_.notify_one();
  }
}

bool ThreadPool::UpdateTaskStatus(const Task::Handle& handle,
                                  Task::Status status) {
  {
    std::lock_guard<std::mutex> lock(mutex_);

    // Check if the task should be cancelled.
    if (cancelled_tasks_.find(handle) != cancelled_tasks_.end()) {
      LOG_S(INFO) << "Task " << handle << " has been cancelled.";
      status = Task::Status::CANCELLED;
    }

    // Update the status record.
    handle_to_status_[handle] = status;
  }

  LOG_IF_S(ERROR, status == Task::Status::FAILED)
      << "Task " << handle << " failed!";

  // Indicate whether we should exit or not.
  return status == Task::Status::RUNNING;
}

void ThreadPool::RunTask(Task* task) {
  // Perform one-time setup.
  LOG_S(1) << "Performing setup for task " << task->GetHandle() << ".";
  Task::Status status = task->SetUp();

  // Run the task.
  while (UpdateTaskStatus(task->GetHandle(), status)) {
    LOG_S(1) << "Running one iteration of task " << task->GetHandle() << ".";
    status = task->RunAtomic();
  }

  // The task is finished. Perform cleanup.
  LOG_S(1) << "Cleaning up task " << task->GetHandle() << ".";
  task->CleanUp();

  // Notify that the task is complete.
  {
    std::lock_guard<std::mutex> lock(mutex_);

    joinable_queue_.push(task->GetHandle());
    ++num_completed_tasks_;
  }
  task_done_.notify_all();
}

Task::Status ThreadPool::GetTaskStatus(const std::shared_ptr<Task>& task) {
  std::lock_guard<std::mutex> lock(mutex_);

  const auto handle_and_status = handle_to_status_.find(task->GetHandle());
  CHECK_S(handle_and_status != handle_to_status_.end())
      << "Attempt to get status of nonexistent thread.";
  return handle_to_status_[task->GetHandle()];
}

void ThreadPool::CancelTask(const std::shared_ptr<Task>& task_handle) {
  std::lock_guard<std::mutex> lock(mutex_);

  LOG_S(INFO) << "Cancelling task " << task_handle << ".";
  cancelled_tasks_.insert(task_handle->GetHandle());
}

void ThreadPool::WaitForCompletion() {
  std::unique_lock<std::mutex> lock(mutex_);

  if (dispatch_queue_.empty() && pool_size_ == 0) {
    // No running or pending tasks.
    return;
  }

  const uint32_t kInitialCompletedTasks = num_completed_tasks_;

  // Wait for a task to finish.
  task_done_.wait(lock, [this, kInitialCompletedTasks]() {
    return num_completed_tasks_ != kInitialCompletedTasks ||
           (dispatch_queue_.empty() && pool_size_ == 0);
  });
}

uint32_t ThreadPool::NumThreads() {
  std::lock_guard<std::mutex> lock(mutex_);
  return pool_size_;
}

}  // namespace thread_pool