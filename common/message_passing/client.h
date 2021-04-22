#ifndef CSCI6780_MESSAGE_PASSING_CLIENT_H
#define CSCI6780_MESSAGE_PASSING_CLIENT_H

#include <google/protobuf/message.h>
#include <sys/socket.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "queue/queue.h"
#include "tasks/receiver_task.h"
#include "tasks/sender_task.h"
#include "thread_pool/thread_pool.h"
#include "types.h"
#include "wire_protocol/wire_protocol.h"

namespace message_passing {

/**
 * @brief Handles sending messages to another node.
 */
class Client {
 public:
  /**
   * @param thread_pool Thread pool to use internally for managing associated
   *    tasks.
   * @param destination The destination node that we will send messages to.
   */
  Client(std::shared_ptr<thread_pool::ThreadPool> thread_pool,
         Endpoint destination);
  ~Client();

  /**
   * @brief Sends a message to another node synchronously. Will block until the
   *    message is sent.
   * @param message The message to send.
   * @return Result of the internal call to `send()`.
   */
  int Send(const google::protobuf::Message& message);

  /**
   * @brief Sends a message to another node asynchronously. Will return
   *    immediately, before the message is sent. Errors will not be reported.
   * @param message The message to send.
   * @return True if it succeeded in dispatching the send request, false
   *    otherwise.
   */
  bool SendAsync(const google::protobuf::Message& message);

  /**
   * @brief Receives a message.
   * @tparam MessageType The type of message we expect.
   * @param message[out] The message that we received.
   * @return True if a message was successfully received and parsed, false
   *    otherwise.
   */
  template <class MessageType>
  bool Receive(MessageType* message) {
    // Make sure we're connected.
    EnsureConnected();

    // Create the parser for the response.
    wire_protocol::MessageParser<MessageType> parser;
    // Add any overflow data from the previous call.
    LOG_S(2) << "Adding " << partial_received_messages_.size()
             << " bytes of overflow data to parser.";
    parser.AddNewData(partial_received_messages_);

    // Receive the response.
    while (!parser.HasCompleteMessage()) {
      const auto kResponse = receive_queue_->Pop();
      if (kResponse.status <= 0) {
        // The receive failed or the endpoint disconnected.
        return false;
      }

      parser.AddNewData(kResponse.message);
    }

    // Save any overflow data to use for the next message.
    partial_received_messages_ = parser.GetOverflow();
    return parser.GetMessage(message);
  }

  /**
   * @brief Sends a request, then waits for a response from the receiver.
   * @tparam ResponseType The type of response message we expect.
   * @param request The request to be sent.
   * @param response[out] The response that we received.
   * @return True if the request was successfully sent and a response was
   *    successfully received, false otherwise.
   */
  template <class ResponseType>
  bool SendRequest(const google::protobuf::Message& request,
                   ResponseType* response) {
    // First, send the request.
    if (Send(request) < 0) {
      return false;
    }

    // Now, receive the response.
    return Receive(response);
  }

 private:
  /**
   * @brief Dispatches a new message send operation.
   * @param message The message to send.
   * @param async Whether to send the message asynchronously.
   * @param id[out] Filled with the chosen message ID, if not null.
   * @return True if the dispatch succeeded, false otherwise.
   */
  bool DispatchSend(const google::protobuf::Message& message, bool async,
                    MessageId* id);

  /**
   * @brief Ensures that we are connected to the server.
   * @return True if it is connected, false if it failed to connect.
   */
  bool EnsureConnected();

  /// The thread pool to use for running tasks.
  std::shared_ptr<thread_pool::ThreadPool> thread_pool_;
  /// The endpoint we are sending messages to.
  Endpoint endpoint_;
  /**
   * @brief Queue containing messages for the sender task to send.
   */
  std::shared_ptr<queue::Queue<SenderTask::SendQueueMessage>> send_queue_;
  /// Queue containing received messages.
  std::shared_ptr<queue::Queue<ReceiverTask::ReceiveQueueMessage>>
      receive_queue_;
  /// The task responsible for sending messages.
  std::shared_ptr<SenderTask> sender_task_ = nullptr;
  /// The task responsible for receiving messages.
  std::shared_ptr<ReceiverTask> receiver_task_ = nullptr;

  /// Current message id.
  std::atomic<MessageId> message_id_ = 0;

  /// Associates send return values with message IDs.
  std::unordered_map<MessageId, int> send_results_{};
  /// Protects access to send_results_;
  std::mutex mutex_{};
  /// Used to wait for changes to `send_results_`.
  std::condition_variable send_results_updated_;

  /// The file descriptor for the client socket.
  int client_fd_ = -1;

  /// Stores partially-received data that hasn't been processed yet.
  std::vector<uint8_t> partial_received_messages_{};
};

}  // namespace message_passing

#endif  // CSCI6780_MESSAGE_PASSING_CLIENT_H
