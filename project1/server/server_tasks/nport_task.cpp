#include "nport_task.h"

#include <sys/socket.h>

#include <cstdio>
#include <iostream>
#include <utility>

#include "agent_task.h"
#include <loguru.hpp>

namespace server_tasks {

thread_pool::Task::Status NPortTask::Listen() {
  loguru::set_thread_name("TPort Thread");
  LOG_F(INFO, "Server is listening for normal commands on port %i.", port_);

  while (true) {
    // Accept a new connection.
    int client_fd = accept(server_fd_, nullptr, nullptr);
    if (client_fd < 0) {
      LOG_F(ERROR, "accept() failed");
      return thread_pool::Task::Status::FAILED;
    }

    LOG_F(INFO, "Normal Port handling new connection from client #%i.",client_fd);

    auto agent_task = std::make_shared<AgentTask>(
        client_fd, active_ids_, read_manager_, write_manager_);
    pool_.AddTask(agent_task);
  }
}

NPortTask::NPortTask(
    std::shared_ptr<CommandIDs> active_ids, uint16_t port,
    std::shared_ptr<server::file_handler::FileAccessManager> read_mgr,
    std::shared_ptr<server::file_handler::FileAccessManager> write_mgr)
    : ServerTask(std::move(active_ids), port),
      read_manager_(std::move(read_mgr)),
      write_manager_(std::move(write_mgr)) {}

}  // namespace server_tasks
