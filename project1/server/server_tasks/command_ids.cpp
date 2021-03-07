#include "command_ids.h"
namespace server_tasks {
    void CommandIDs::Insert(uint16_t id) {
        std::lock_guard<std::mutex> guard(mutex_);
        command_ids_.push_back(id);
    }
    void CommandIDs::Delete(uint16_t id) {
        std::lock_guard<std::mutex> guard(mutex_);
        command_ids_.remove(id);
    }
    bool CommandIDs::Contains(uint16_t id) {
        std::lock_guard<std::mutex> guard(mutex_);
        std::list<uint16_t>::iterator find = std::find(command_ids_.begin(),
                                                       command_ids_.end(),id);
        if (find != command_ids_.end()){
            return true;
        }
        return false;
    }
}

