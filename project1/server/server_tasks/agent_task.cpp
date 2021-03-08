#include "agent_task.h"

#include <utility>

namespace server_tasks {

    AgentTask::AgentTask(int id, std::shared_ptr<CommandIDs> cmds,
                         std::shared_ptr<server::file_handler::FileAccessManager> read_mgr,
                         std::shared_ptr<server::file_handler::FileAccessManager> write_mgr)
            : client_fd_(id), active_commands_(std::move(cmds)),
              read_manager_(std::move(read_mgr)), write_manager_(std::move(write_mgr)) {}

    AgentTask::AgentTask(int id, std::shared_ptr<CommandIDs> cmds)
    : client_fd_(id), active_commands_(cmds) {}

    thread_pool::Task::Status AgentTask::SetUp() {

        // if this is an agent for a normal command, these members should be initialized
        if (read_manager_ && write_manager_) {
            // give the agent a unique file handler with the shared access managers
            auto fh = std::make_unique<server::file_handler::ThreadSafeFileHandler>(read_manager_, write_manager_);
            agent_ = std::make_unique<server::Agent>(client_fd_,std::move(fh),std::move(active_commands_));
        } else {
            // no need to give an Agent a file handler for termination commands
            agent_ = std::make_unique<server::Agent>(client_fd_,std::move(active_commands_));
        }

        return Status::RUNNING;
    }

    thread_pool::Task::Status AgentTask::RunAtomic() {
        if (agent_->Handle()) {
            return Status::DONE;
        }
        return Status::FAILED;
    }


}// namespace server_tasks
