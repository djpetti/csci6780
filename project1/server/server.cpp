#include "server.h"

#include <netinet/in.h>
#include <memory>
#include <loguru.cpp>
#include "../thread_pool/thread_pool.h"
#include "server_tasks/nport_task.h"
#include "server_tasks/tport_task.h"

namespace server {

    void Server::FtpService(uint16_t nPort, uint16_t tPort) {
        LOG_F(INFO, "FtpService now starting.");
        thread_pool::ThreadPool pool;
        std::shared_ptr<server_tasks::CommandIDs> active_ids;

        // pass active command list to nPortTask and tPortTask
        auto nPortTask = std::make_shared<server_tasks::NPortTask>(active_ids,nPort,
                                                                   read_manager_,write_manager_);
        auto tPortTask = std::make_shared<server_tasks::TPortTask>(active_ids,tPort,
                                                                   read_manager_,write_manager_);

        pool.AddTask(nPortTask);
        pool.AddTask(tPortTask);
        LOG_F(INFO, "Normal Port and Termination port added to thread pool.");
        while (pool.GetTaskStatus(nPortTask) == thread_pool::Task::Status::RUNNING ||
               pool.GetTaskStatus(tPortTask) == thread_pool::Task::Status::RUNNING) {
            pool.WaitForCompletion();
        }

    }
}  // namespace server