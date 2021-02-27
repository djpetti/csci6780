
#include "task.h"

namespace thread_pool {

std::atomic<uint32_t> Task::current_id_ = 1;

uint32_t thread_pool::Task::GetHandle() const {
  return id_;
}

Task::Task() : id_(current_id_++) {}

}  // namespace thread_pool