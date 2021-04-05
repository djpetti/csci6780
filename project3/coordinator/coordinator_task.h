/**
 * @file Task implementation of coordinator.
 */

#ifndef CSCI6780_COORDINATOR_TASK_H
#define CSCI6780_COORDINATOR_TASK_H
#include "thread_pool/task.h"
#include "thread_pool/thread_pool.h"
#include "coordinator.h"
namespace coordinator {
/**
 * @class Task implementation of coordinator.
 */
class CoordinatorTask : public thread_pool::Task {
 public:
  /**
   *
   * @param participant_fd The socket
   * @param hostname The hostname.
   * @param msg_mgr The message manager.
   * @param registrar The registrar.
   * @param msg_queue The message queue.
   * @param msg_log The message log.
   */
  CoordinatorTask(int participant_fd, std::string hostname,
              std::shared_ptr<MessengerManager> msg_mgr,
              std::shared_ptr<Registrar> registrar,
              std::shared_ptr<queue::Queue<MessageLog::Message>> msg_queue,
              std::shared_ptr<MessageLog> msg_log);

  thread_pool::Task::Status RunAtomic() final;
  thread_pool::Task::Status SetUp() final;

 private:

  /// The coordinator.
  std::unique_ptr<Coordinator> coordinator_;

  /// This CoordinatorTask's client
  int participant_fd_;

  /// The participant's machine name.
  std::string hostname_;

  /// The Messenger Manager
  std::shared_ptr<MessengerManager> msg_mgr_;

  /// The Registrar
  std::shared_ptr<Registrar> registrar_;

  /// The Message Queue
  std::shared_ptr<queue::Queue<MessageLog::Message>> msg_queue_;

  /// The Message Log
  std::shared_ptr<MessageLog> msg_log_;


};
}
#endif  // CSCI6780_COORDINATOR_TASK_H
