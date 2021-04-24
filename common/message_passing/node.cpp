#include "node.h"

#include <utility>

namespace message_passing {

Node::Node(std::shared_ptr<thread_pool::ThreadPool> thread_pool)
    : thread_pool_(std::move(thread_pool)),
      receive_queue_(
          std::make_shared<queue::Queue<ReceiverTask::ReceiveQueueMessage>>()) {
}

Node::~Node() {
  // Cancel all the tasks we created.
  for (const auto& task : receiver_tasks_) {
    thread_pool_->CancelTask(task);
  }
}

void Node::StartReceiverTask(int socket_fd, const Endpoint& endpoint) {
  LOG_S(1) << "Starting receiver task for " << endpoint.hostname << ":"
           << endpoint.port << " on socket " << socket_fd << ".";

  auto receiver_task =
      std::make_shared<ReceiverTask>(socket_fd, receive_queue_, endpoint);
  receiver_tasks_.push_back(receiver_task);
  thread_pool_->AddTask(receiver_task);
}

std::shared_ptr<thread_pool::ThreadPool> Node::thread_pool() {
  return thread_pool_;
}

std::shared_ptr<queue::Queue<ReceiverTask::ReceiveQueueMessage>>
Node::receive_queue() {
  return receive_queue_;
}

}  // namespace message_passing
