#include "server.h"

#include <loguru.hpp>
#include <utility>

#include "wire_protocol/wire_protocol.h"

namespace message_passing {

using wire_protocol::Serialize;

Server::Server(std::shared_ptr<thread_pool::ThreadPool> thread_pool,
               uint16_t listen_port)
    : Node(std::move(thread_pool)),
      server_task_(std::make_shared<ServerTask>(
          listen_port, Server::thread_pool(), receive_queue(),
          [this](const Endpoint& endpoint,
                 std::shared_ptr<queue::Queue<SenderTask::SendQueueMessage>>
                     send_queue) {
            // Save the send queue for the new client.
            std::lock_guard<std::mutex> lock(send_queue_mutex_);
            send_queues_[endpoint] = std::move(send_queue);
          },

          [this](MessageId id, int status) {
            {
              std::lock_guard<std::mutex> lock(send_results_mutex_);
              send_results_[id] = status;
            }
            // Notify everyone waiting on this.
            send_results_updated_.notify_all();
          })) {
  // Start the server task.
  Server::thread_pool()->AddTask(server_task_);
}

Server::~Server() {
  LOG_S(INFO) << "Server is exiting, cancelling the server task.";
  thread_pool()->CancelTask(server_task_);

  // Actually wait for the task to cancel, so we don't leave open ports before
  // returning.
  thread_pool()->WaitForCompletion(server_task_);
}

int Server::Send(const google::protobuf::Message& message,
                 const Endpoint& destination) {
  MessageId message_id;
  if (!DispatchSend(message, destination, false, &message_id)) {
    // Failed to dispatch the send.
    return -1;
  }

  // Wait for the send to finish.
  {
    std::unique_lock<std::mutex> lock(send_results_mutex_);
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

bool Server::SendAsync(const google::protobuf::Message& message,
                       const Endpoint& destination) {
  return DispatchSend(message, destination, true, nullptr);
}

bool Server::DispatchSend(const google::protobuf::Message& message,
                          const Endpoint& endpoint, bool async, MessageId* id) {
  if (!EnsureConnected()) {
    return false;
  }

  // Prepare the message to send on the queue.
  SenderTask::SendQueueMessage queue_message{++message_id_, {}, async};
  // Serialize the message.
  if (!Serialize(message, &queue_message.message)) {
    LOG_S(ERROR) << "Message serialization failed.";
    return false;
  }

  if (id != nullptr) {
    *id = queue_message.message_id;
  }

  {
    std::lock_guard<std::mutex> lock(send_queue_mutex_);

    // Get the queue to use.
    auto endpoint_and_queue = send_queues_.find(endpoint);
    if (endpoint_and_queue == send_queues_.end()) {
      LOG_S(ERROR) << "Cannot send to endpoint " << endpoint.hostname << ":"
                   << endpoint.port << " because it is not connected.";
      return false;
    }

    // Send the message.
    endpoint_and_queue->second->Push(queue_message);
  }

  return true;
}

std::unordered_set<Endpoint, EndpointHash> Server::GetConnected() {
  std::lock_guard<std::mutex> lock(send_queue_mutex_);

  std::unordered_set<Endpoint, EndpointHash> connected;
  for (const auto& kEndpointAndQueue : send_queues_) {
    connected.insert(kEndpointAndQueue.first);
  }

  return connected;
}

bool Server::EnsureConnected() {
  // Make sure the server task is actually running.
  if (thread_pool()->GetTaskStatus(server_task_) !=
      ServerTask::Status::RUNNING) {
    LOG_S(WARNING) << "Server task is no longer running.";
    return false;
  }

  return true;
}

}  // namespace message_passing