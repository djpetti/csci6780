#include "client.h"

#include <unistd.h>

#include <chrono>
#include <functional>
#include <loguru.hpp>
#include <utility>

#include "utils.h"
#include "wire_protocol/wire_protocol.h"

namespace message_passing {
namespace {

/// Maximum number of seconds to wait for pending sends to complete at exit.
const auto kSendTimeout = std::chrono::seconds(5);

}  // namespace

using wire_protocol::Serialize;

Client::Client(std::shared_ptr<thread_pool::ThreadPool> thread_pool,
               Endpoint destination)
    : Node(std::move(thread_pool)),
      endpoint_(std::move(destination)),
      send_queue_(
          std::make_shared<queue::Queue<SenderTask::SendQueueMessage>>()) {}

Client::~Client() {
  // Wait for any pending sends to complete.
  LOG_S(1) << "Waiting for pending send operations to finish...";
  if (!send_queue_->WaitUntilEmpty(kSendTimeout)) {
    LOG_S(WARNING) << "Send operations did not complete in time.";
  }

  // Cancel the tasks we added to the thread pool.
  LOG_S(1) << "Cancelling sender and receiver tasks...";
  if (sender_task_ != nullptr) {
    thread_pool()->CancelTask(sender_task_);
    thread_pool()->WaitForCompletion(sender_task_);
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

  return client_fd_ >= 0;
}

}  // namespace message_passing