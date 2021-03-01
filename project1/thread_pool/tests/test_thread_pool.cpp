/**
 * @file Unit tests for `thread_pool`.
 */

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

#include "../task.h"
#include "../thread_pool.h"
#include "gtest/gtest.h"

namespace thread_pool::tests {
namespace {

/// Timeout to use when waiting for a thread to complete, in seconds.
const uint32_t kTaskTimeout = 5;

/**
 * @brief Waits until a task in the pool completes.
 * @param pool The pool that the task is in.
 * @param task The task to wait on.
 * @return True if the task completed, false if the operation timed out.
 */
bool WaitForTaskCompletion(ThreadPool* pool,
                           const std::shared_ptr<Task>& task) {
  const auto kStartTime = std::chrono::steady_clock::now();
  while (pool->GetTaskStatus(task) == Task::Status::RUNNING) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    if (std::chrono::steady_clock::now() - kStartTime >
        std::chrono::seconds(kTaskTimeout)) {
      // Task took too long to complete.
      return false;
    }
  }

  return true;
}

/**
 * @class A simple task that succeeds immediately.
 */
class BasicTask : public Task {
 public:
  Status RunAtomic() final {
    result_ = 2 + 2;
    return Status::DONE;
  }

  /**
   * @return The result from this task.
   */
  [[nodiscard]] int GetResult() const { return result_; }

 private:
  /// Result of our very complex calculation.
  int result_ = 0;
};

/**
 * @brief A task that runs forever.
 */
class InfiniteTask : public Task {
 public:
  Status RunAtomic() final {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    return Status::RUNNING;
  }
};

/**
 * @brief A task with non-trivial SetUp and CleanUp methods.
 */
class TaskWithInit : public Task {
 public:
  /// Whether we ran SetUp.
  bool ran_set_up = false;
  /// Whether we ran CleanUp.
  bool ran_clean_up = false;
  /// Whether we ran RunAtomic.
  bool ran_loop = false;

  /**
   * @param fail_start_up Whether to simulate a task failure in the StartUp
   *    method.
   */
  explicit TaskWithInit(bool fail_start_up = false)
      : fail_start_up_(fail_start_up) {}

  Status SetUp() final {
    ran_set_up = true;

    return fail_start_up_ ? Status::FAILED : Status::RUNNING;
  }

  Status RunAtomic() final {
    ran_loop = true;
    return Status::DONE;
  }

  void CleanUp() final { ran_clean_up = true; }

 private:
  /// If true, simulate a failure in the start-up process.
  bool fail_start_up_{};
};

}  // namespace

/**
 * @test Tests that we can successfully run a single task.
 */
TEST(ThreadPool, SingleBasicTask) {
  // Arrange.
  ThreadPool pool;
  auto task = std::make_shared<BasicTask>();

  // Act.
  pool.AddTask(task);

  // Assert.
  // Wait for the task to finish.
  ASSERT_TRUE(WaitForTaskCompletion(&pool, task));

  // The status should be set correctly.
  EXPECT_EQ(Task::Status::DONE, pool.GetTaskStatus(task));
  // The result should be correct.
  EXPECT_EQ(4U, task->GetResult());
}

/**
 * @test Tests that we can run many tasks concurrently.
 */
TEST(ThreadPool, ManyBasicTasks) {
  // Arrange.
  ThreadPool pool;
  std::vector<std::shared_ptr<BasicTask>> tasks;
  for (uint32_t i = 0; i < 100; ++i) {
    tasks.push_back(std::make_shared<BasicTask>());
  }

  // Act.
  for (const auto& task : tasks) {
    pool.AddTask(task);
  }

  // Assert.
  // Wait for all tasks to complete.
  for (const auto& task : tasks) {
    ASSERT_TRUE(WaitForTaskCompletion(&pool, task));

    // The task should have been successful.
    EXPECT_EQ(Task::Status::DONE, pool.GetTaskStatus(task));
    // The result should be correct.
    EXPECT_EQ(4U, task->GetResult());
  }
}

/**
 * @test Tests that we can cancel a task.
 */
TEST(ThreadPool, TaskCancellation) {
  // Arrange.
  ThreadPool pool;
  auto task1 = std::make_shared<InfiniteTask>();
  auto task2 = std::make_shared<InfiniteTask>();

  // Act.
  pool.AddTask(task1);
  pool.AddTask(task2);

  // Assert.
  // Tasks should both initially be running.
  EXPECT_EQ(Task::Status::RUNNING, pool.GetTaskStatus(task1));
  EXPECT_EQ(Task::Status::RUNNING, pool.GetTaskStatus(task2));

  // Cancel one of them.
  pool.CancelTask(task1);
  ASSERT_TRUE(WaitForTaskCompletion(&pool, task1));

  // It should now be marked as cancelled.
  EXPECT_EQ(Task::Status::CANCELLED, pool.GetTaskStatus(task1));
  // The other should still be running.
  EXPECT_EQ(Task::Status::RUNNING, pool.GetTaskStatus(task2));

  // Cancel the other one.
  pool.CancelTask(task2);
  ASSERT_TRUE(WaitForTaskCompletion(&pool, task2));

  // Both should be marked as cancelled.
  EXPECT_EQ(Task::Status::CANCELLED, pool.GetTaskStatus(task1));
  EXPECT_EQ(Task::Status::CANCELLED, pool.GetTaskStatus(task2));
}

/**
 * @test Tests that destroying the pool cancels running tasks.
 */
TEST(ThreadPool, DtorCancelsTasksSmoke) {
  // Arrange.
  ThreadPool pool;
  auto task = std::make_shared<InfiniteTask>();

  // Act.
  pool.AddTask(task);
  // Exiting this scope should destroy the pool and cancel the task.
}

/**
 * @test Tests that the pool does not exceed a thread limit.
 */
TEST(ThreadPool, LimittedThreads) {
  // Arrange.
  // Make a pool with a maximum of 2 threads.
  ThreadPool pool(2);

  auto task1 = std::make_shared<InfiniteTask>();
  auto task2 = std::make_shared<InfiniteTask>();
  auto task3 = std::make_shared<InfiniteTask>();

  // Act.
  pool.AddTask(task1);
  pool.AddTask(task2);
  pool.AddTask(task3);

  // Assert.
  // Despite there being three tasks, it should only have created two threads.
  EXPECT_EQ(2U, pool.NumThreads());
}

/**
 * @test Tests that SetUp and CleanUp procedures are run.
 */
TEST(ThreadPool, SetUpCleanUp) {
  // Arrange.
  ThreadPool pool;
  auto task = std::make_shared<TaskWithInit>();

  // Act.
  pool.AddTask(task);

  // Assert.
  // Wait for the task to finish.
  ASSERT_TRUE(WaitForTaskCompletion(&pool, task));

  // The task should have succeeded.
  EXPECT_EQ(Task::Status::DONE, pool.GetTaskStatus(task));

  // The task lifecycle should have been adhered to.
  EXPECT_TRUE(task->ran_set_up);
  EXPECT_TRUE(task->ran_loop);
  EXPECT_TRUE(task->ran_clean_up);
}

/**
 * @test Tests that the pool handles it when task setup fails.
 */
TEST(ThreadPool, SetUpFailed) {
  // Arrange.
  ThreadPool pool;
  // This task will fail in the SetUp method.
  auto task = std::make_shared<TaskWithInit>(true);

  // Act.
  pool.AddTask(task);

  // Assert.
  // Wait for the task to finish.
  ASSERT_TRUE(WaitForTaskCompletion(&pool, task));

  // The task should have failed.
  EXPECT_EQ(Task::Status::FAILED, pool.GetTaskStatus(task));

  // The task lifecycle should have been adhered to.
  EXPECT_TRUE(task->ran_set_up);
  // It should never run the main loop if the setup fails.
  EXPECT_FALSE(task->ran_loop);
  EXPECT_TRUE(task->ran_clean_up);
}

}  // namespace thread_pool::tests