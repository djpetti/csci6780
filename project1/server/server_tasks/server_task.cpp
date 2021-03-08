#include "server_task.h"
namespace server_tasks {

    ServerTask::ServerTask(std::shared_ptr<CommandIDs> active_ids, uint16_t port)
    : port_(port),active_ids_(active_ids){}

    thread_pool::Task::Status ServerTask::SetUp()  {
        // Create the socket.
        const auto kAddress = MakeAddress(port_);

        server_fd_ = SetUpSocket(kAddress);
        if (server_fd_ < 0) {
            return thread_pool::Task::Status::FAILED;
        } else {return thread_pool::Task::Status::RUNNING;}
    }

    thread_pool::Task::Status ServerTask::RunAtomic() {
        return Listen();
    }

    struct sockaddr_in ServerTask::MakeAddress(uint16_t port) {
        struct sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        return address;
    }

    int ServerTask::SetUpSocket(const struct sockaddr_in &address) {
        // Open a TCP socket.
        const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0) {
            perror("Failed to create server socket");
            return -1;
        }

        // Allow the server to re-bind to this port if it was restarted quickly.
        const int option = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option,
                       sizeof(option))) {
            perror("Failed to set socket options");
            // This is not a fatal error.
        }

        // Bind to the port.
        if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) < 0) {
            perror("bind() failed on server socket");
            return -1;
        }
        if (listen(server_fd, kMaxQueueSize_) < 0) {
            perror("listen() failed on server socket");
            return -1;
        }

        return server_fd;
    }
}

