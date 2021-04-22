#ifndef CSCI6780_RECEIVER_TASK_H
#define CSCI6780_RECEIVER_TASK_H

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

#include "../types.h"
#include "queue/queue.h"
#include "thread_pool/task.h"

namespace message_passing {

/**
 * @brief Task that is responsible for reading messages from a socket.
 */
class ReceiverTask : public thread_pool::Task {
 public:
  /// Queue message containing messages to be sent.
  struct ReceiveQueueMessage {
    /// The serialized message.
    std::vector<uint8_t> message;

    /// recv() call status associated with this message.
    int status;
  };

  /**
   * @param receive_fd The file descriptor to send on.
   * @param receive_queue The queue that messages to send will be received on.
   * @param send_callback Callback function to call with the message ID and the
   *    result of `send()` every time a message is sent.
   */
  ReceiverTask(int receive_fd,
      std::shared_ptr<queue::Queue<ReceiveQueueMessage>> receive_queue);

  Status RunAtomic() final;

 private:
  /// Size of chunks to receive messages in.
  static constexpr uint32_t kReceiveChunkSize = 1024;

  /// File descriptor to receive messages on.
  int receive_fd_;
  /// Buffer to use for partial received messages.
  std::vector<uint8_t> received_message_buffer_{};

  /// Queue to receive messages on.
  std::shared_ptr<queue::Queue<ReceiveQueueMessage>> receive_queue_;
};

}  // namespace message_passing

#endif  // CSCI6780_RECEIVER_TASK_H
