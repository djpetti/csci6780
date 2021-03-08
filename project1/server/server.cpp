#include "server.h"

#include <netinet/in.h>
#include <memory>
#include <loguru.cpp>
#include "../thread_pool/thread_pool.h"
#include "server_tasks/nport_task.h"
#include "server_tasks/tport_task.h"

namespace server {

    void Server::FtpService(uint16_t nPort, uint16_t tPort) {
        thread_pool::ThreadPool pool;
        std::shared_ptr<server_tasks::CommandIDs> active_ids;

        // pass active command list to nPortTask and tPortTask
        auto nPortTask = std::make_shared<server_tasks::NPortTask>(std::move(active_ids),nPort,
                                                                   std::move(read_manager_),std::move(write_manager_));
        auto tPortTask = std::make_shared<server_tasks::TPortTask>(std::move(active_ids),tPort,
                                                                   std::move(read_manager_),std::move(write_manager_));

        pool.AddTask(nPortTask);
        pool.AddTask(tPortTask);

        while (pool.GetTaskStatus(nPortTask) != thread_pool::Task::Status::DONE ||
               pool.GetTaskStatus(tPortTask) != thread_pool::Task::Status::DONE) {
            pool.WaitForCompletion();
        }

    }
}  // namespace server