#include "server.h"

#include <netinet/in.h>

#include <loguru.cpp>
#include <memory>

#include "server_tasks/nport_task.h"
#include "server_tasks/tport_task.h"

namespace server {

Server::Server()
    : read_manager_(std::make_shared<file_handler::FileAccessManager>()),
      write_manager_(std::make_shared<file_handler::FileAccessManager>()) {}

void Server::FtpService(uint16_t nPort, uint16_t tPort) {
  thread_pool::ThreadPool pool;
  auto active_ids = std::make_shared<server_tasks::CommandIDs>();

  // pass active command list to nPortTask and tPortTask
  auto nPortTask = std::make_shared<server_tasks::NPortTask>(
      active_ids, nPort, read_manager_, write_manager_);
  auto tPortTask = std::make_shared<server_tasks::TPortTask>(active_ids, tPort);

  pool.AddTask(nPortTask);
  pool.AddTask(tPortTask);

  while (pool.GetTaskStatus(nPortTask) == thread_pool::Task::Status::RUNNING ||
         pool.GetTaskStatus(tPortTask) == thread_pool::Task::Status::RUNNING) {
    pool.WaitForCompletion();
  }
}

}  // namespace server