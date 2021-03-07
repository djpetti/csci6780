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
         */
        virtual thread_pool::Task::Status Listen();

        thread_pool::Task::Status SetUp() override;
        thread_pool::Task::Status RunAtomic() override;
        void SetActiveCommandIDs(std::shared_ptr<CommandIDs> cmd_ids);
        void SetPort(uint16_t port);

    protected:

        /**
         * @brief
         * @param port
         * @return
         */
        struct sockaddr_in MakeAddress(uint16_t port);
        int SetUpSocket(const struct sockaddr_in &address);

        ///The thread pool.
        thread_pool::ThreadPool pool_;

        ///The port # for terminate commands.
        uint16_t port_;

        ///The list containing the active command IDs
        std::shared_ptr<CommandIDs> active_ids_;

        int server_fd_;

        /// Maximum queue size to use for listening on the server socket.
        uint8_t kMaxQueueSize_= 5;

    };
}
#endif //PROJECT1_SERVER_TASK_H
