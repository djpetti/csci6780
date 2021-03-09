#include "tport_task.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdint>
#include <cstdio>
#include <utility>

#include "../../thread_pool/thread_pool.h"
#include "agent_task.h"
namespace server_tasks {

thread_pool::Task::Status TPortTask::Listen() {
  std::cout << "Server is listening for termination commands on port " << port_
            << "." << std::endl;

  while (true) {
    // Accept a new connection.
    int client_fd = accept(server_fd_, nullptr, nullptr);
    if (client_fd < 0) {
      perror("accept() failed");
      return thread_pool::Task::Status::FAILED;
    }

    std::cout << "Handling new connection from client #" << client_fd << "."
              << std::endl;

    auto agent_task = std::make_shared<AgentTask>(client_fd, active_ids_);

    pool_.AddTask(agent_task);
  }
}

TPortTask::TPortTask(std::shared_ptr<CommandIDs> active_ids, uint16_t port)
    : ServerTask(std::move(active_ids), port) {}

}  // namespace server_tasks
