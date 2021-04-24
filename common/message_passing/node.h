#ifndef CSCI6780_MESSAGE_PASSING_NODE_H
#define CSCI6780_MESSAGE_PASSING_NODE_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <loguru.hpp>
#include <memory>
#include <queue>
#include <vector>

#include "queue/queue.h"
#include "tasks/receiver_task.h"
#include "thread_pool/thread_pool.h"
#include "types.h"
#include "wire_protocol/wire_protocol.h"

namespace message_passing {

/**
 * @brief Common superclass for all message-passing nodes.
 */
class Node {
 public:
  /**
   * @param thread_pool Thread pool to use internally for managing associated
   *    tasks.
   */
  explicit Node(std::shared_ptr<thread_pool::ThreadPool> thread_pool);
  virtual ~Node();

  /**
   * @brief Waits for a new message from a client.
   * @tparam MessageType The type of message that we will receive.
   * @param message[out] Set to the received message.
   * @param source[out] Set to the source of the message, if provided. If
   *    it is nullptr, it will be ignored.
   * @return True if it successfully received and parsed the message,
   *    false otherwise.
   */
  template <class MessageType>
  bool Receive(MessageType* message, Endpoint* source = nullptr) {
    return DoReceive(
        [this](ReceiverTask::ReceiveQueueMessage* message) {
          *message = receive_queue_->Pop();
          return true;
        },
        message, source);
  }

  /**
   * @brief Receive overload that allows for a timeout.
   * @tparam MessageType The type of message that we will receive.
   * @tparam Rep The underlying numeric type for the timeout duration.
   * @tparam Period The underlying period type for the timeout duration.
   * @param timeout The timeout.
   * @param message[out] Set to the received message.
   * @param source[out] Set to the source of the message, if provided. If it
   *    is nullptr, it will be ignored.
   * @return True if it successfully received and parsed the message, false
   *    otherwise or if it timed out.
   */
  template <class MessageType, class Rep, class Period>
  bool Receive(const std::chrono::duration<Rep, Period>& timeout,
               MessageType* message, Endpoint* source = nullptr) {
    return DoReceive(
        [this, &timeout](ReceiverTask::ReceiveQueueMessage* message) {
          return receive_queue_->PopTimed(timeout, message);
        },
        message, source);
  }

 protected:
  /**
   * @brief Starts a new task for receiving messages on a socket.
   * @param socket_fd The socket to receive messages on.
   * @param endpoint The endpoint that this socket is connected to.
   */
  void StartReceiverTask(int socket_fd, const Endpoint& endpoint);

  /**
   * @return The thread pool to use for this class.
   */
  std::shared_ptr<thread_pool::ThreadPool> thread_pool();
  /**
   * @return The receive queue to use for this class.
   */
  std::shared_ptr<queue::Queue<ReceiverTask::ReceiveQueueMessage>>
  receive_queue();

  /**
   * @brief Ensures that any sockets are initialized and we are properly
   *    connected.
   * @return True if it is connected, false if it failed to connect.
   */
  virtual bool EnsureConnected() = 0;

 private:
  /**
   * @brief Internal receive function.
   * @tparam MessageType The type of message that we will receive.
   * @param pop_queue Function to use to pop the next message from the queue.
   * @param message[out] Set to the received message.
   * @param source[out] Set to the source of the message, if provided. If
   *    it is nullptr, it will be ignored.
   * @return True if it successfully received and parsed the message,
   *    false otherwise.
   */
  template <class MessageType>
  bool DoReceive(
      const std::function<bool(ReceiverTask::ReceiveQueueMessage*)>& pop_queue,
      MessageType* message, Endpoint* source) {
    // Make sure we're connected.
    if (!EnsureConnected()) {
      return false;
    }

    // Create the parser for the response.
    wire_protocol::MessageParser<MessageType> parser;

    // Messages that were read off the queue but not yet processed.
    auto un_processed_messages = un_processed_messages_;
    un_processed_messages_ = {};

    // Receive the response.
    Endpoint reading_endpoint;
    bool set_reading_endpoint = false;
    while (!parser.HasCompleteMessage()) {
      // Get the next message.
      ReceiverTask::ReceiveQueueMessage response;
      if (!un_processed_messages.empty()) {
        response = un_processed_messages.front();
        un_processed_messages.pop();
      } else if (!pop_queue(&response)) {
        // Receive timed out.
        return false;
      }

      // Make sure we're only processing messages from the same endpoint.
      if (!set_reading_endpoint) {
        reading_endpoint = response.endpoint;
        set_reading_endpoint = true;
      } else if (response.endpoint != reading_endpoint) {
        // Save this message, but ignore it.
        un_processed_messages_.push(response);
        continue;
      }

      if (response.status <= 0) {
        // The receive failed or the endpoint disconnected.
        return false;
      }

      parser.AddNewData(response.message);
    }

    if (source != nullptr) {
      *source = reading_endpoint;
    }
    // Save any overflow data to use for the next message.
    if (parser.HasOverflow()) {
      const auto& overflow = parser.GetOverflow();
      un_processed_messages_.push(
          {overflow, reading_endpoint, static_cast<int>(overflow.size())});
    }
    return parser.GetMessage(message);
  }

  /// Internal thread pool to use for managing tasks.
  std::shared_ptr<thread_pool::ThreadPool> thread_pool_;

  /// Queue for receiving messages on.
  std::shared_ptr<queue::Queue<ReceiverTask::ReceiveQueueMessage>>
      receive_queue_;
  /// Keeps track of all the ReceiverTasks that it created.
  std::vector<std::shared_ptr<ReceiverTask>> receiver_tasks_{};

  /// A buffer of all messages that have been received but not yet processed.
  std::queue<ReceiverTask::ReceiveQueueMessage> un_processed_messages_;
};

}  // namespace message_passing

#endif  // CSCI6780_MESSAGE_PASSING_NODE_H
