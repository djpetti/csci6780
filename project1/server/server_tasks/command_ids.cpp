#include "command_ids.h"
#include <loguru.hpp>
namespace server_tasks {
    void CommandIDs::Insert(uint16_t id) {
        LOG_F(INFO, "Inserting command #%i into Active Command Directory.", id);
        std::lock_guard<std::mutex> guard(mutex_);
        command_ids_.insert(id);
    }
    void CommandIDs::Delete(uint16_t id) {
        LOG_F(INFO, "Deleting command #%i into Active Command Directory.", id);
        std::lock_guard<std::mutex> guard(mutex_);
        command_ids_.erase(id);
    }
    bool CommandIDs::Contains(uint16_t id) {
        std::lock_guard<std::mutex> guard(mutex_);
        const auto& find = command_ids_.find(id);
        return find != command_ids_.end();
    }
}

