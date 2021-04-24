#ifndef CSCI6780_MESSAGE_PASSING_SERVER_H
#define CSCI6780_MESSAGE_PASSING_SERVER_H

#include <google/protobuf/message.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

#include "node.h"
#include "queue/queue.h"
#include "tasks/receiver_task.h"
#include "tasks/sender_task.h"
#include "tasks/server_task.h"
#include "thread_pool/thread_pool.h"
#include "types.h"

namespace message_passing {

/**
 * @brief Handles running a server.
 */
class Server : public Node {
 public:
  /**
   * @param listen_port The port for the server to listen on.
   * @param thread_pool Thread pool to use internally for managing associated
   *    tasks.
   */
  Server(std::shared_ptr<thread_pool::ThreadPool> thread_pool,
         uint16_t listen_port);
  ~Server() override;

  /**
   * @brief Sends a message to a connected client synchronously. Will block
   *    until the message is sent.
   * @param message The message to send.
   * @param destination The connected node to send the message to.
   * @return Result of the internal call to `send()`. Will also return -1
   *    if the specified node is not connected.
   */
  int Send(const google::protobuf::Message& message,
           const Endpoint& destination);

  /**
   * @brief Sends a message to a connected client asynchronously. Will return
   *    immediately, before the message is sent. Errors will not be reported.
   * @param message The message to send.
   * @param destination The connected node to send the message to.
   * @return True if it succeeded in dispatching the send request, false
   *    otherwise.
   */
  bool SendAsync(const google::protobuf::Message& message,
                 const Endpoint& destination);

  /**
   * @return The set of all clients that are currently connected.
   */
  std::unordered_set<Endpoint, EndpointHash> GetConnected();

 protected:
  bool EnsureConnected() final;

 private:
  /**
   * @brief Dispatches a new message send operation.
   * @param message The message to send.
   * @param async Whether to send the message asynchronously.
   * @param id[out] Filled with the chosen message ID, if not null.
   * @return True if the dispatch succeeded, false otherwise.
   */
  bool DispatchSend(const google::protobuf::Message& message,
                    const Endpoint& endpoint, bool async, MessageId* id);

  /// Maps endpoints to send queues for that particular endpoint.
  std::unordered_map<Endpoint,
                     std::shared_ptr<queue::Queue<SenderTask::SendQueueMessage>>,
                     EndpointHash>
      send_queues_{};
  /// Mutex to protect access to the send queues.
  std::mutex send_queue_mutex_{};

  /// The task that actually implements the server.
  std::shared_ptr<ServerTask> server_task_;

  /// Associates send return values with message IDs.
  std::unordered_map<MessageId, int> send_results_{};
  /// Protects access to send_results_;
  std::mutex send_results_mutex_{};
  /// Used to wait for changes to `send_results_`.
  std::condition_variable send_results_updated_;

  /// Current message id.
  std::atomic<MessageId> message_id_ = 0;
};

}  // namespace message_passing

#endif  // CSCI6780_MESSAGE_PASSING_SERVER_H
