#include "command_ids.h"
namespace server_tasks {
    void CommandIDs::Insert(uint16_t id) {
        std::lock_guard<std::mutex> guard(mutex_);
        command_ids_.insert(id);
    }
    void CommandIDs::Delete(uint16_t id) {
        std::lock_guard<std::mutex> guard(mutex_);
        command_ids_.erase(id);
    }
    bool CommandIDs::Contains(uint16_t id) {
        std::lock_guard<std::mutex> guard(mutex_);
        std::unordered_set<uint16_t>::iterator find = std::find(command_ids_.begin(),
                                                       command_ids_.end(),id);
        return find != command_ids_.end();
    }
}

