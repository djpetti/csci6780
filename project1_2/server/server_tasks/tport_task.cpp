#include "tport_task.h"
#include "agent_task.h"

#include <sys/socket.h>

#include <cstdint>
#include <utility>
#include <loguru.hpp>
namespace server_tasks {

    thread_pool::Task::Status TPortTask::Listen() {
        loguru::set_thread_name("TPort Thread");
        LOG_F(INFO, "Server is listening for termination commands on port %i.", port_);

        while (true) {

            // Accept a new connection.
            int client_fd = accept(server_fd_, nullptr, nullptr);
            if (client_fd < 0) {
                LOG_F(ERROR, "accept() failed");
                return thread_pool::Task::Status::FAILED;
            }

            LOG_F(INFO, "Termination Port handling new connection from client #%i.",client_fd);
            auto agent_task = std::make_shared<AgentTask>(client_fd, active_ids_);


    pool_.AddTask(agent_task);
  }
}

TPortTask::TPortTask(std::shared_ptr<CommandIDs> active_ids, uint16_t port)
    : ServerTask(std::move(active_ids), port) {}

}  // namespace server_tasks
