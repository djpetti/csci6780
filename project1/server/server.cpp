#include "server.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdio>
#include <iostream>
#include <memory>
#include <thread>

#include "agent.h"
#include "file_handler/file_handler.h"
#include "../thread_pool/thread_pool.h"
#include "server_tasks/nport_task.h"
#include "server_tasks/tport_task.h"

namespace server {

    void Server::FtpService(uint16_t nPort, uint16_t tPort) {

        thread_pool::ThreadPool pool;

        std::shared_ptr<server_tasks::CommandIDs> active_ids;
        auto nPortTask = std::make_shared<server_tasks::NPortTask>();
        auto tPortTask = std::make_shared<server_tasks::TPortTask>();

        // pass active command list to nPortTask and tPortTask
        nPortTask->SetActiveCommandIDs(active_ids);
        tPortTask->SetActiveCommandIDs(active_ids);

        // pass file access managers to nPortTask
        nPortTask->SetFileAccessManagers(read_manager_,write_manager_);

        nPortTask->SetPort(nPort);
        tPortTask->SetPort(tPort);

        pool.AddTask(nPortTask);
        pool.AddTask(tPortTask);

    }
}  // namespace server