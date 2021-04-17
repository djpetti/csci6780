#ifndef CSCI6780_MESSAGE_SENDER_H
#define CSCI6780_MESSAGE_SENDER_H

#include <google/protobuf/message.h>

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "queue/queue.h"
#include "tasks/sender_task.h"
#include "thread_pool/thread_pool.h"
#include "types.h"

namespace message_passing {

/**
 * @brief Handles sending messages to another node.
 */
class MessageSender {
 public:
  /**
   * @param thread_pool Thread pool to use internally for managing associated
   *    tasks.
   */
  explicit MessageSender(std::shared_ptr<thread_pool::ThreadPool> thread_pool);
  ~MessageSender();

  /**
   * @brief Sends a message to another node synchronously. Will block until the
   *    message is sent.
   * @param message The message to send.
   * @param destination The destination node.
   * @return Result of the internal call to `send()`.
   */
  int Send(const google::protobuf::Message& message,
           const Endpoint& destination);

  /**
   * @brief Sends a message to another node asynchronously. Will return
   *    immediately, before the message is sent. Errors will not be reported.
   * @param message The message to send.
   * @param destination The destination node.
   * @return True if it succeeded in dispatching the send request, false
   *    otherwise.
   */
  bool SendAsync(const google::protobuf::Message& message,
                 const Endpoint& destination);

 private:
  /**
   * @brief Dispatches a new message send operation.
   * @param message The message to send.
   * @param destination The destination node to send it to.
   * @param async Whether to send the message asynchronously.
   * @param id[out] Filled with the chosen message ID, if not null.
   * @return True if the dispatch succeeded, false otherwise.
   */
  bool DispatchSend(const google::protobuf::Message& message,
                    const Endpoint& destination, bool async, MessageId* id);

  /// The thread pool to use for running tasks.
  std::shared_ptr<thread_pool::ThreadPool> thread_pool_;
  /**
   * @brief Queue containing messages for the sender task to send.
   */
  std::shared_ptr<queue::Queue<SenderTask::SendQueueMessage>> send_queue_;
  /// The task responsible for sending messages.
  std::shared_ptr<SenderTask> sender_task_;

  /// Current message id.
  std::atomic<MessageId> message_id_ = 0;

  /// Associates return send return values with message IDs.
  std::unordered_map<MessageId, int> send_results_{};
  /// Protects access to send_results_;
  std::mutex mutex_{};
  /// Used to wait for changes to `send_results_`.
  std::condition_variable send_results_updated_;
};

}  // namespace message_passing

#endif  // CSCI6780_MESSAGE_SENDER_H
