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

}  // namespace queue::tests