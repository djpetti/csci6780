#include "sender_task.h"

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

SenderTask::SenderTask(
    int send_fd, std::shared_ptr<queue::Queue<SendQueueMessage>> send_queue,
    SendCallback send_callback)
    : send_fd_(send_fd),
      send_queue_(std::move(send_queue)),
      send_callback_(std::move(send_callback)) {}

Task::Status message_passing::SenderTask::RunAtomic() {
  // Read the next message from the queue.
  SendQueueMessage message;
  if (!send_queue_->PopTimed(kQueueTimeout, &message)) {
    // Nothing new on the queue.
    return Task::Status::RUNNING;
  }

  // Attempt to send.
  const ssize_t kSendResult =
      send(send_fd_, message.message.data(), message.message.size(), 0);
  if (kSendResult < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // This is merely a timeout. We can try again later. Add the message
      // back to the queue.
      LOG_S(INFO) << "Send timed out for message " << message.message_id
                  << ". Will retry.";
      send_queue_->Push(message);
      return Task::Status::RUNNING;
    }

    // General failure to send.
    LOG_S(ERROR) << "Socket error: " << std::strerror(errno);
  }

  if (!message.send_async) {
    // Run the callback to indicate the send result.
    send_callback_(message.message_id, static_cast<int>(kSendResult));
  }
  return Task::Status::RUNNING;
}

int SenderTask::GetFd() const { return send_fd_; }

}  // namespace message_passing
