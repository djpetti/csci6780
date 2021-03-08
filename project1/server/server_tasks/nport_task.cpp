#include "nport_task.h"
#include "agent_task.h"
#include "../../thread_pool/thread_pool.h"

#include <sys/socket.h>
#include <cstdio>
#include <iostream>

namespace server_tasks {

    thread_pool::Task::Status NPortTask::Listen() {


        std::cout << "Server is listening for normal commands on port " << port_ << "." << std::endl;

        while (true) {

            // Accept a new connection.
            int client_fd = accept(server_fd_, nullptr, nullptr);
            if (client_fd < 0) {
                perror("accept() failed");
                return thread_pool::Task::Status::FAILED;
            }

            std::cout << "Handling new connection from client #" << client_fd << "." << std::endl;

            auto agent_task = std::make_shared<AgentTask>(client_fd, std::move(active_ids_),
                                                          std::move(read_manager_), std::move(write_manager_));
            pool_.AddTask(agent_task);

        }


    }
}// namespace server_tasks
