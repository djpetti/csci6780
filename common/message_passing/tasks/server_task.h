#ifndef CSCI6780_MESSAGE_PASSING_SERVER_TASK_H
#define CSCI6780_MESSAGE_PASSING_SERVER_TASK_H

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_set>

#include "../types.h"
#include "queue/queue.h"
#include "receiver_task.h"
#include "sender_task.h"
#include "socket_task_interface.h"
#include "thread_pool/thread_pool.h"

namespace message_passing {

/**
 * @brief Task that's responsible for listening
 *  on a server socket and handling clients.
 */
class ServerTask : public ISocketTask {
 public:
  /**
   * @brief Callback that gets run whenever a new client connects.
   *    It is called with the endpoint of the new connection, and the
   *    queue that can be used to send messages to this client.
   */
  using NewClientCallback = std::function<void(
      const Endpoint&,
      std::shared_ptr<queue::Queue<SenderTask::SendQueueMessage>>)>;

  /**
   * @param listen_port The port that the server should listen on.
   * @param thread_pool The thread pool to use for handling server-related
   *    tasks.
   * @param receive_queue The queue that we want received messages to be pushed
   *    onto.
   * @param new_client_callback The callback to run whenever a new client
   *    connects.
   * @param send_callback The callback to run whenever a message gets sent
   *    to any client.
   */
  ServerTask(uint16_t listen_port,
             std::shared_ptr<thread_pool::ThreadPool> thread_pool,
             std::shared_ptr<queue::Queue<ReceiverTask::ReceiveQueueMessage>>
                 receive_queue,
             NewClientCallback new_client_callback,
             SenderTask::SendCallback send_callback);
  ~ServerTask() override = default;

  Status SetUp() final;
  Status RunAtomic() final;
  void CleanUp() final;
  [[nodiscard]] int GetFd() const final;

 private:
  /**
   * @brief Cleans up the tasks associated with disconnected clients.
   */
  void CloseDisconnected();

  /// The port to listen on.
  uint16_t listen_port_;
  /// The thread pool to use for internal tasks.
  std::shared_ptr<thread_pool::ThreadPool> thread_pool_;
  /// The queue that we want to receive messages on.
  std::shared_ptr<queue::Queue<ReceiverTask::ReceiveQueueMessage>>
      receive_queue_;
  /// All the tasks that we've started so far.
  std::unordered_set<std::shared_ptr<ISocketTask>> tasks_;

  /// Callback to run when a client connects.
  NewClientCallback new_client_callback_;
  /// Callback to run when a message is sent.
  SenderTask::SendCallback send_callback_;

  /// Socket we are listening on.
  int server_socket_ = -1;
};

}  // namespace message_passing

#endif  // CSCI6780_MESSAGE_PASSING_SERVER_TASK_H
