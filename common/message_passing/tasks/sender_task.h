#ifndef CSCI6780_SENDER_TASK_H
#define CSCI6780_SENDER_TASK_H

#include <functional>
#include <memory>
#include <unordered_map>

#include "../types.h"
#include "queue/queue.h"
#include "thread_pool/task.h"

namespace message_passing {

/**
 * @brief Task that is responsible for reading
 *  messages off a queue and sending them.
 */
class SenderTask : public thread_pool::Task {
 public:
  /// Type alias for the send callback function.
  using SendCallback = std::function<void(MessageId, int)>;

  /// Queue message containing messages to be sent.
  struct SendQueueMessage {
    /// Unique ID for the message.
    MessageId message_id;

    /// The serialized message.
    std::vector<uint8_t> message;
    /// The destination node.
    Endpoint endpoint;

    /// Whether to send this message asynchronously.
    bool send_async;
  };

  /**
   * @param send_queue The queue that messages to send will be received on.
   * @param send_callback Callback function to call with the message ID and the
   *    result of `send()` every time a message is sent.
   */
  SenderTask(std::shared_ptr<queue::Queue<SendQueueMessage>> send_queue,
             SendCallback send_callback);

  Status RunAtomic() final;
  void CleanUp() final;

 private:
  /**
   * @brief Gets the connection to a particular endpoint, and opens a new
   *    one if it does not exist.
   * @param endpoint The endpoint to connect to.
   * @return The socket FD associated with this connection.
   */
  int GetConnection(const Endpoint& endpoint);

  /// Queue to receive messages on.
  std::shared_ptr<queue::Queue<SendQueueMessage>> send_queue_;
  /// Callback to run when a send completes.
  SendCallback send_callback_;

  /// Maps endpoints to corresponding connections.
  std::unordered_map<Endpoint, int, EndpointHash> open_connections_;
};

}  // namespace message_passing

#endif  // CSCI6780_SENDER_TASK_H
