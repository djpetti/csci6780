/**
 * @file Unit tests for `queue`.
 */

#include <thread>
#include <vector>

#include "../queue.h"
#include "gtest/gtest.h"

namespace queue::tests {

/**
 * @test Tests that we can push and pop from the queue with a single thread.
 */
TEST(Queue, PushPopSingleThread) {
  // Arrange.
  Queue<int> queue;

  // Act.
  queue.Push(42);
  const int got_element = queue.Pop();

  // Assert.
  EXPECT_EQ(42, got_element);
}

/**
 * @test Tests that we can push and pop from the queue with multiple threads.
 */
TEST(Queue, PushPopTwoThreads) {
  // Arrange.
  Queue<int> queue;

  // Stores the elements that we read from the queue.
  std::vector<int> got_elements;

  // Act.
  // Create the threads.
  std::thread producer([&queue]() {
    for (int i = 0; i < 1000; ++i) {
      queue.Push(i);
    }
  });
  std::thread consumer([&queue, &got_elements]() {
    for (int i = 0; i < 1000; ++i) {
      got_elements.push_back(queue.Pop());
    }
  });

  producer.join();
  consumer.join();

  // Assert.
  // It should have gotten all the elements in the right order.
  for (int i = 0; i < 1000; ++i) {
    EXPECT_EQ(i, got_elements[i]);
  }
}

/**
 * @test Tests that we can push and pop from the queue with multiple threads
 * when a maximum length is set.
 */
TEST(Queue, PushPopTwoThreadsMaxLength) {
  // Arrange.
  Queue<int> queue(5);

  // Stores the elements that we read from the queue.
  std::vector<int> got_elements;

  // Act.
  // Create the threads.
  std::thread producer([&queue]() {
    for (int i = 0; i < 1000; ++i) {
      queue.Push(i);
    }
  });
  std::thread consumer([&queue, &got_elements]() {
    for (int i = 0; i < 1000; ++i) {
      got_elements.push_back(queue.Pop());
    }
  });

  producer.join();
  consumer.join();

  // Assert.
  // It should have gotten all the elements in the right order.
  for (int i = 0; i < 1000; ++i) {
    EXPECT_EQ(i, got_elements[i]);
  }
}

/**
 * @test Tests that `Empty` works.
 */
TEST(Queue, Empty) {
  // Arrange.
  Queue<int> queue;

  // Act and assert.
  EXPECT_TRUE(queue.Empty());

  // Add something to the queue.
  queue.Push(42);
  // It should not be empty anymore.
  EXPECT_FALSE(queue.Empty());
}

/**
 * @test Tests that `PopTimed` works.
 */
TEST(Queue, PopTimeout) {
  // Arrange.
  Queue<int> queue;

  const auto kTimeout = std::chrono::milliseconds(100);

  // Act.
  int got_element;
  queue.Push(42);
  EXPECT_TRUE(queue.PopTimed(kTimeout, &got_element));

  // Next pop should time out.
  EXPECT_FALSE(queue.PopTimed(kTimeout, &got_element));

  // Assert.
  EXPECT_EQ(42, got_element);
}

/**
 * @test Tests that `WaitUntilEmpty` works.
 */
TEST(Queue, WaitUntilEmpty) {
  // Arrange.
  Queue<int> queue;

  // Push some elements onto the queue initially.
  queue.Push(1);
  queue.Push(2);

  // Act.
  // In another thread, wait for the queue to be empty.
  std::thread waiter_thread([&]() { queue.WaitUntilEmpty(); });

  // Pop the elements off the queue.
  queue.Pop();
  queue.Pop();

  // Assert.
  // The waiting should now be complete.
  waiter_thread.join();
}

/**
 * @test Tests that `WaitUntilEmpty` works with timeouts.
 */
TEST(Queue, WaitUntilEmptyTimeout) {
  // Arrange.
  Queue<int> queue;

  const auto kTimeout = std::chrono::milliseconds(100);

  // Push some elements onto the queue initially.
  queue.Push(1);
  queue.Push(2);

  // Act.
  // Initially, it should timeout.
  const bool kWaitResult1 = queue.WaitUntilEmpty(kTimeout);

  // Pop one off.
  queue.Pop();
  // It should still not be empty.
  const bool kWaitResult2 = queue.WaitUntilEmpty(kTimeout);

  // Pop the next one off.
  queue.Pop();
  // It should now return immediately.
  const bool kWaitResult3 = queue.WaitUntilEmpty(kTimeout);

  // Assert.
  EXPECT_FALSE(kWaitResult1);
  EXPECT_FALSE(kWaitResult2);
  EXPECT_TRUE(kWaitResult3);
}

}  // namespace queue::tests