#include "sender_task.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <loguru.hpp>
#include <utility>

namespace message_passing {
namespace {

/// Timeout to use when reading queue messages.
const auto kQueueTimeout = std::chrono::seconds(1);
/// Timeout in seconds for socket operations.
constexpr uint32_t kSocketTimeout = 1;

struct sockaddr_in MakeAddress(uint16_t port) {
  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  return address;
}

int SetUpSocket(const sockaddr_in &address, const std::string &hostname) {
  int sock;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    LOG_S(ERROR) << "Socket creation error: " << std::strerror(errno);
    return -1;
  }

  if (inet_pton(AF_INET, hostname.c_str(),
                (struct sockaddr *)&address.sin_addr) <= 0) {
    LOG_S(ERROR) << "inet_pton: " << std::strerror(errno);
    return -1;
  }

  // Set a timeout.
  struct timeval timeout {};
  timeout.tv_sec = kSocketTimeout;
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout,
             sizeof(timeout));

  if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    LOG_S(ERROR) << "Connection Failed: " << std::strerror(errno);
    return -1;
  }

  return sock;
}

}  // namespace

using thread_pool::Task;

SenderTask::SenderTask(
    std::shared_ptr<queue::Queue<SendQueueMessage>> send_queue,
    SendCallback send_callback)
    : send_queue_(std::move(send_queue)),
      send_callback_(std::move(send_callback)) {}

Task::Status message_passing::SenderTask::RunAtomic() {
  // Read the next message from the queue.
  SendQueueMessage message;
  if (!send_queue_->PopTimed(kQueueTimeout, &message)) {
    // Nothing new on the queue.
    return Task::Status::RUNNING;
  }

  // Get the associated connection.
  const int kSockFd = GetConnection(message.endpoint);
  if (kSockFd == -1) {
    // We were unable to open a connection. This counts as a failure.
    send_callback_(message.message_id, -1);
    return Task::Status::RUNNING;
  }

  // Attempt to send.
  const ssize_t kSendResult =
      send(kSockFd, message.message.data(), message.message.size(), 0);
  if (kSendResult < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      // This is merely a timeout. We can try again later.
      return Task::Status::RUNNING;
    }
    if (errno == EPIPE || errno == ECONNRESET) {
      // Socket failed because remote end disconnected. In this case,
      // it's worth trying to reconnect.
      LOG_S(INFO) << "Connection to " << message.endpoint.hostname
                  << " closed by other end. Will try to reconnect.";

      // First, remove the existing connection.
      open_connections_.erase(message.endpoint);
      // Add the message back to the queue, where it will be processed later.
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

int SenderTask::GetConnection(const Endpoint &endpoint) {
  const auto &kConnection = open_connections_.find(endpoint);
  if (kConnection != open_connections_.end()) {
    // We have an existing connection.
    return kConnection->second;
  }

  // We need to open a new connection.
  LOG_S(INFO) << "Opening a new connection to " << endpoint.hostname << ":"
              << endpoint.port << ".";
  const auto kAddress = MakeAddress(endpoint.port);
  const int kSockFd = SetUpSocket(kAddress, endpoint.hostname);
  if (kSockFd == -1) {
    // Failed to open the connection for some reason.
    return kSockFd;
  }

  // Otherwise, add it to the map of open connections.
  open_connections_[endpoint] = kSockFd;

  return kSockFd;
}

void SenderTask::CleanUp() {
  // Gracefully close all open connections.
  LOG_S(INFO) << "Closing " << open_connections_.size() << " open connections.";

  for (const auto &connection : open_connections_) {
    close(connection.second);
  }
}

}  // namespace message_passing
