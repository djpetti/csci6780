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

            auto agent_task = std::make_shared<AgentTask>();

            // pass client file descriptor to the agent task
            agent_task->SetClientFD(client_fd);

            // pass file access managers to the agent task
            agent_task->SetFileAccessManagers(read_manager_,write_manager_);

            // pass the active commands to the agent task
            agent_task->SetActiveCommands(active_ids_);


            pool_.AddTask(agent_task);

        }
    }

    void NPortTask::SetFileAccessManagers(std::shared_ptr<server::file_handler::FileAccessManager> read_mgr,
                                          std::shared_ptr<server::file_handler::FileAccessManager> write_mgr) {
        read_manager_ = read_mgr;
        write_manager_ = write_mgr;
    }


} //namespace server_tasks