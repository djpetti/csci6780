#ifndef CSCI6780_MESSAGE_PASSING_SOCKET_TASK_H
#define CSCI6780_MESSAGE_PASSING_SOCKET_TASK_H

#include "thread_pool/task.h"

/**
 * @brief Interface for a task that manages a socket.
 */
class ISocketTask : public thread_pool::Task {
 public:
  /**
   * @return The file descriptor of the associated socket.
   */
  [[nodiscard]] virtual int GetFd() const = 0;
};

#endif  // CSCI6780_MESSAGE_PASSING_SOCKET_TASK_H
