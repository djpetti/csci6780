
#include "task.h"

namespace thread_pool {

std::atomic<uint32_t> Task::current_id_ = 1;

Task::Task() : id_(current_id_++) {}

uint32_t thread_pool::Task::GetHandle() const { return id_; }

Task::Status Task::SetUp() { return Status::RUNNING; }

void Task::CleanUp() {}

}  // namespace thread_pool