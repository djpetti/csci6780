
#include "task.h"

namespace thread_pool {

uint32_t Task::current_id_ = 0;

uint32_t thread_pool::Task::GetId() {
  return current_id_++;
}

}  // namespace thread_pool