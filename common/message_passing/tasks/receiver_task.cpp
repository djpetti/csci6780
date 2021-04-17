#include "receiver_task.h"

#include <sys/socket.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <loguru.hpp>
#include <utility>

namespace message_passing {
namespace {

/// Timeout to use when reading queue messages.
const auto kQueueTimeout = std::chrono::seconds(1);

}  // namespace

using thread_pool::Task;

ReceiverTask::ReceiverTask(
    int receive_fd,
    std::shared_ptr<queue::Queue<ReceiveQueueMessage>> receive_queue)
    : receive_fd_(receive_fd), receive_queue_(std::move(receive_queue)) {}

Task::Status message_passing::ReceiverTask::RunAtomic() {
  ReceiveQueueMessage message;

  // Receive the next message.
  received_message_buffer_.resize(kReceiveChunkSize);
  const ssize_t kReceiveResult =
      recv(receive_fd_, received_message_buffer_.data(), kReceiveChunkSize, 0);
  if (kReceiveResult <= 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // This is merely a timeout. We can try again later.
      return Task::Status::RUNNING;
    }

    // General failure to receive.
    LOG_S(ERROR) << "Socket error: " << std::strerror(errno);
  } else {
    // Resize the buffer to the actual amount of content received so we can
    // tell where the actual data ends.
    received_message_buffer_.resize(kReceiveResult);
    message.message = received_message_buffer_;
  }

  // Add the message to the queue.
  message.status = static_cast<int>(kReceiveResult);
  receive_queue_->Push(message);

  // If the receive fails, we fail the task, because otherwise we'll probably
  // just get stuck in an infinite loop.
  return kReceiveResult >= 0 ? Task::Status::RUNNING : Task::Status::FAILED;
}

}  // namespace message_passing
