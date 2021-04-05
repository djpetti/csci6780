/**
 * @file Sets up socket and deploys coordinator's for newly connected clients.
 */
#ifndef CSCI6780_COORDINATOR_DRIVER_H
#define CSCI6780_COORDINATOR_DRIVER_H

#include "coordinator_task.h"
#include "thread_pool/task.h"
#include "thread_pool/thread_pool.h"
#include "messenger_manager.h"
#include "registrar.h"
#include "message_log.h"
#include "queue/queue.h"
#include "connected_participants.h"

namespace coordinator {
using coordinator::MessengerManager;
using coordinator::Registrar;
using coordinator::MessageLog;
using Timestamp = std::chrono::steady_clock::time_point;
using Duration = std::chrono::steady_clock::duration;

/**
 * Deploys a CoordinatorTask for newly connected clients.
 */
class CoordinatorDriver {
 public:
  /**
   * @brief Listens for participant connections.
   * @param port The port to listen on.
   * @param threshold The missed messages time threshold.
   */
  [[noreturn]] void Start(uint16_t port, Duration threshold);
 private:
  /// The data structures shared by coordinators.
  std::shared_ptr<ConnectedParticipants> participants_;
  std::shared_ptr<MessageLog> message_log_;
  std::shared_ptr<MessengerManager> messenger_manager_;
  std::shared_ptr<Registrar> registrar_;
  std::shared_ptr<queue::Queue<MessageLog::Message>> message_queue_;

  /// Maximum queue size to use for listening on the server socket.
  uint8_t static constexpr kMaxQueueSize_ = 5;

  /// The thread pool.
  thread_pool::ThreadPool pool_;

  /**
   * Helper functions for creating sockets.
   *
   */
  static struct sockaddr_in MakeAddress(int port);
  static int SetUpSocket(const struct sockaddr_in &address);
  int CreateSocket(int port);

};
}
#endif  // CSCI6780_COORDINATOR_DRIVER_H
