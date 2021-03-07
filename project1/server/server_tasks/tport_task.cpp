#include "tport_task.h"
#include "../../thread_pool/thread_pool.h"

#include <sys/socket.h>
#include <cstdio>
#include <netinet/in.h>
#include <cstdint>
namespace server_tasks {

    thread_pool::Task::Status TPortTask::Listen() {
        std::cout << "Server is listening for termination commands on port " << port_ << "." << std::endl;

        while (true) {

            // Accept a new connection.
            int client_fd = accept(server_fd_, nullptr, nullptr);
            if (client_fd < 0) {
                perror("accept() failed");
                return thread_pool::Task::Status::FAILED;
            }

            std::cout << "Handling new connection from client #" << client_fd << "." << std::endl;

            //create new TerminationTask
            //Add Task to ThreadPool

            //pool_.AddTask()
            //std::thread agent_thread(HandleClient, client_fd);

            //agent_thread.detach();
        }
    }


}

