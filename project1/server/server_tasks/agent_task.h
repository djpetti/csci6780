/**
 * @File Task wrapper for the Agent Class.
 */

#ifndef PROJECT1_AGENT_TASK_H
#define PROJECT1_AGENT_TASK_H

#include "../../thread_pool/task.h"
#include "../../thread_pool/thread_pool.h"
#include "../file_handler/file_handler.h"
#include "agent.h"
#include "command_ids.h"

namespace server_tasks {
    class AgentTask : public thread_pool::Task {
    public:

        AgentTask(int id, std::shared_ptr<CommandIDs> cmds,
                  std::shared_ptr<server::file_handler::FileAccessManager> read_mgr,
                  std::shared_ptr<server::file_handler::FileAccessManager> write_mgr);

        AgentTask(int id, std::shared_ptr<CommandIDs> cmds);

        thread_pool::Task::Status RunAtomic() final;
        thread_pool::Task::Status SetUp() final;

    private:

        ///This AgentTask's client.
        int client_fd_;

        ///Active Commands @Note: To be inherited from the ServerTask, to be supplied to the agent
        std::shared_ptr<CommandIDs> active_commands_;

        ///This AgentTask's Agent.
        std::unique_ptr<server::Agent> agent_;

        ///The file access managers. @note To be inherited from NPortTask.
        std::shared_ptr<server::file_handler::FileAccessManager> read_manager_;
        std::shared_ptr<server::file_handler::FileAccessManager> write_manager_;

    };
}
#endif //PROJECT1_AGENT_TASK_H
