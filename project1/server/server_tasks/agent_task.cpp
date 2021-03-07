#include "agent_task.h"

namespace server_tasks {

    void AgentTask::SetClientFD(int fd) {
        client_fd_ = fd;
    }

    void AgentTask::SetActiveCommands(std::shared_ptr<CommandIDs>cmds) {
        active_commands_ = cmds;
    }

    void AgentTask::SetFileAccessManagers(std::shared_ptr<server::file_handler::FileAccessManager> read_mgr,
                                          std::shared_ptr<server::file_handler::FileAccessManager> write_mgr) {
        read_manager_ = read_mgr;
        write_manager_ = write_mgr;
    }

    thread_pool::Task::Status AgentTask::SetUp() {
        auto fh = std::make_unique<server::file_handler::ThreadSafeFileHandler>(read_manager_,write_manager_);
        agent_ = new server::Agent(client_fd_, std::move(fh));
        return Status::RUNNING;
    }

    thread_pool::Task::Status AgentTask::RunAtomic() {
        if (agent_->Handle()) {
            return Status::DONE;
        }
        return Status::FAILED;
    }



}// namespace server_tasks
