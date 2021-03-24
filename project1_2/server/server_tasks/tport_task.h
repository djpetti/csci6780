/**
 * @File Task for monitoring port for 'Terminate' Commands
 */

#ifndef PROJECT1_TPORT_TASK_H
#define PROJECT1_TPORT_TASK_H

#include <utility>

#include "thread_pool/task.h"
#include "thread_pool/thread_pool.h"
#include "../file_handler/file_access_manager.h"
#include "../file_handler/file_handler.h"
#include "command_ids.h"
#include "server_task.h"

namespace server_tasks {

class TPortTask : public ServerTask {
 public:
  /**
   * @brief Initializes all necessary information to run this task.
   * @param active_ids The set of active command IDs
   * @param port The port to bind to
   */
  TPortTask(std::shared_ptr<CommandIDs> active_ids, uint16_t port);

  /**
   * Listens for Terminate Commands
   * @return The client status.
   */
  thread_pool::Task::Status Listen() final;
};
}  // namespace server_tasks
#endif  // PROJECT1_TPORT_TASK_H
