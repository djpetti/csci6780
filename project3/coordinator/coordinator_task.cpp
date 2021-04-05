/**
 * @file Implementation of CoordinatorTask
 */
#include "coordinator_task.h"

#include <loguru.hpp>
#include <string>
#include <utility>

namespace coordinator {
CoordinatorTask::CoordinatorTask(
    int participant_fd, std::string hostname,
    std::shared_ptr<MessengerManager> msg_mgr,
    std::shared_ptr<Registrar> registrar,
    std::shared_ptr<queue::Queue<MessageLog::Message>> msg_queue,
    std::shared_ptr<MessageLog> msg_log)
    : participant_fd_(participant_fd),
      hostname_(hostname),
      msg_mgr_(msg_mgr),
      registrar_(registrar),
      msg_queue_(msg_queue),
      msg_log_(msg_log) {}
thread_pool::Task::Status CoordinatorTask::SetUp() {
  std::string thread_name = "Agent Thread #" + std::to_string(participant_fd_);
  loguru::set_thread_name(thread_name.c_str());
  coordinator_ = std::make_unique<Coordinator>(
      participant_fd_, hostname_, msg_mgr_, registrar_, msg_queue_, msg_log_);

  return Status::RUNNING;
}

thread_pool::Task::Status CoordinatorTask::RunAtomic() {
  if (coordinator_->Coordinate()) {
    return Status::DONE;
  }
  return Status::FAILED;
}
}  // namespace coordinator
