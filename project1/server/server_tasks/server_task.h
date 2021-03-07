/**
 * @File Represents an abstract task that a server would have (i.e. a task dealing with sockets)
 */

#ifndef PROJECT1_SERVER_TASK_H
#define PROJECT1_SERVER_TASK_H

#include "../../thread_pool/task.h"
#include "../../thread_pool/thread_pool.h"
#include "command_ids.h"
#include <sys/socket.h>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <memory>
#include <cstdint>

namespace server_tasks {
    class ServerTask : public thread_pool::Task {
    public:

        /**
         * @brief Represents how this ServerTask will listen on a socket.
         * @return The status of the thread pool.
         * @note This will be called in RunAtomic.
         */
        virtual thread_pool::Task::Status Listen();

        thread_pool::Task::Status SetUp() override;
        thread_pool::Task::Status RunAtomic() override;

        /**
         * @brief Sets the list of active commands for this server task.
         * @param active_ids The list of active command ID's
         */
        void SetActiveCommandIDs(std::shared_ptr<CommandIDs> active_ids);

        /**
         * @brief Sets the port that this server task will bind to.
         * @param port
         */
        void SetPort(uint16_t port);

    protected:

        /**
         * @brief Helper function to make address structure for socket creation
         * @param port, the port # to bind to
         * @return the address structure for socket creation
         */
        struct sockaddr_in MakeAddress(uint16_t port);

        /**
         * @brief Helper function to open a socket and bind to port.
         * @param address the address structure
         * @return the file descriptor for the socket
         */
        int SetUpSocket(const struct sockaddr_in &address);

        ///The thread pool.
        thread_pool::ThreadPool pool_;

        ///The port # for terminate commands.
        uint16_t port_;

        ///The file descriptor for the server
        int server_fd_;

        /// Maximum queue size to use for listening on the server socket.
        uint8_t kMaxQueueSize_= 5;

        ///The list containing the active command IDs
        std::shared_ptr<CommandIDs> active_ids_;
    };

}
#endif //PROJECT1_SERVER_TASK_H
