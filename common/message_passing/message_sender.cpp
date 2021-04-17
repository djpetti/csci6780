#include "message_sender.h"

#include <functional>
#include <loguru.hpp>
#include <utility>

#include "wire_protocol/wire_protocol.h"

namespace message_passing {

using std::placeholders::_1;
using std::placeholders::_2;

using wire_protocol::Serialize;

MessageSender::MessageSender(
    std::shared_ptr<thread_pool::ThreadPool> thread_pool)
    : thread_pool_(std::move(thread_pool)),
      send_queue_(
          std::make_shared<queue::Queue<SenderTask::SendQueueMessage>>()),
      sender_task_(std::make_shared<SenderTask>(
          send_queue_, [this](MessageId id, int status) {
            {
              std::lock_guard<std::mutex> lock(mutex_);
              send_results_[id] = status;
            }
            // Notify everyone waiting on this.
            send_results_updated_.notify_all();
          })) {
  // Start the SenderTask.
  thread_pool_->AddTask(sender_task_);
}

MessageSender::~MessageSender() {
  // Cancel the task we added to the thread pool.
  LOG_S(1) << "Cancelling sender task...";
  thread_pool_->CancelTask(sender_task_);
}

int MessageSender::Send(const google::protobuf::Message& message,
                        const Endpoint& destination) {
  MessageId message_id;
  if (!DispatchSend(message, destination, false, &message_id)) {
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

bool MessageSender::SendAsync(const google::protobuf::Message& message,
                              const Endpoint& destination) {
  return DispatchSend(message, destination, true, nullptr);
}

bool MessageSender::DispatchSend(const google::protobuf::Message& message,
                                 const Endpoint& destination, bool async,
                                 MessageId* id) {
  // Prepare a message to send on the queue.
  SenderTask::SendQueueMessage queue_message{
      ++message_id_, {}, destination, async};
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

}  // namespace message_passing