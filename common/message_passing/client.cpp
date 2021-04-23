#include "client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <functional>
#include <loguru.hpp>
#include <utility>

#include "wire_protocol/wire_protocol.h"

namespace message_passing {
namespace {

/// Timeout in seconds for socket operations.
constexpr uint32_t kSocketTimeout = 1;

/**
 * @brief Creates an address structure to use for the socket.
 * @param port The port that we want to connect to.
 * @return The address structure that it created.
 */
struct sockaddr_in MakeAddress(uint16_t port) {
  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  return address;
}

/**
 * @brief Connects the socket to a remote server.
 * @param address The address structure specifying what to connect to.
 * @param hostname The hostname to connect to.
 * @return The FD of the client socket, or -1 on failure.
 */
int SetUpSocket(const sockaddr_in& address, const std::string& hostname) {
  int sock;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    LOG_S(ERROR) << "Socket creation error: " << std::strerror(errno);
    return -1;
  }

  if (inet_pton(AF_INET, hostname.c_str(),
                (struct sockaddr*)&address.sin_addr) <= 0) {
    LOG_S(ERROR) << "inet_pton: " << std::strerror(errno);
    return -1;
  }

  // Set a timeout.
  struct timeval timeout {};
  timeout.tv_sec = kSocketTimeout;
  timeout.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout,
             sizeof(timeout));

  if (connect(sock, (struct sockaddr*)&address, sizeof(address)) < 0) {
    LOG_S(ERROR) << "Connection Failed: " << std::strerror(errno);
    return -1;
  }

  return sock;
}

}  // namespace

using wire_protocol::Serialize;

Client::Client(std::shared_ptr<thread_pool::ThreadPool> thread_pool,
               Endpoint destination)
    : Node(std::move(thread_pool)),
      endpoint_(std::move(destination)),
      send_queue_(
          std::make_shared<queue::Queue<SenderTask::SendQueueMessage>>()) {}

Client::~Client() {
  // Cancel the tasks we added to the thread pool.
  LOG_S(1) << "Cancelling sender and receiver tasks...";
  if (sender_task_ != nullptr) {
    thread_pool()->CancelTask(sender_task_);
  }

  // Close the socket.
  close(client_fd_);
}

int Client::Send(const google::protobuf::Message& message) {
  MessageId message_id;
  if (!DispatchSend(message, false, &message_id)) {
    // Failed to dispatch the send.
    return -1;
  }

  // Wait for the send to finish.
  {
    std::unique_lock<std::mutex> lock(mutex_);
    if (send_results_.find(message_id) == send_results_.end()) {
      // Wait for the send to finish.
      send_results_updated_.wait(lock, [this, message_id]() {
        return send_results_.find(message_id) != send_results_.end();
      });
    }

    const int kResult = send_results_[message_id];
    // Erase this result so it doesn't get read twice.
    send_results_.erase(message_id);

    return kResult;
  }
}

bool Client::SendAsync(const google::protobuf::Message& message) {
  return DispatchSend(message, true, nullptr);
}

bool Client::DispatchSend(const google::protobuf::Message& message, bool async,
                          MessageId* id) {
  // Make sure we are connected.
  if (!EnsureConnected()) {
    return false;
  }

  // Prepare a message to send on the queue.
  SenderTask::SendQueueMessage queue_message{++message_id_, {}, async};
  // Serialize the message.
  if (!Serialize(message, &queue_message.message)) {
    // Failed to serialize the message.
    LOG_S(ERROR) << "Message serialization failed.";
    return false;
  }

  if (id != nullptr) {
    *id = queue_message.message_id;
  }

  // Send the queue message.
  send_queue_->Push(queue_message);

  return true;
}

bool Client::EnsureConnected() {
  if (client_fd_ < 0) {
    // Connect to the server.
    LOG_S(INFO) << "Connecting to " << endpoint_.hostname << ":"
                << endpoint_.port << "...";
    const auto kAddress = MakeAddress(endpoint_.port);
    client_fd_ = SetUpSocket(kAddress, endpoint_.hostname);

    // Create the task for sending messages.
    sender_task_ = std::make_shared<SenderTask>(
        client_fd_, send_queue_, [this](MessageId id, int status) {
          {
            std::lock_guard<std::mutex> lock(mutex_);
            send_results_[id] = status;
          }
          // Notify everyone waiting on this.
          send_results_updated_.notify_all();
        });
    thread_pool()->AddTask(sender_task_);

    // Create the task for receiving messages.
    StartReceiverTask(client_fd_, endpoint_);
  }

  return client_fd_;
}

}  // namespace message_passing