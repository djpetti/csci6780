/**
 * @File Task wrapper for the Agent Class.
 */

#ifndef PROJECT1_AGENT_TASK_H
#define PROJECT1_AGENT_TASK_H

#include "../../thread_pool/task.h"
#include "../../thread_pool/thread_pool.h"
#include "../file_handler/file_handler.h"
#include "../agent.h"
#include "command_ids.h"

namespace server_tasks {
    class AgentTask : public thread_pool::Task {
    public:
        thread_pool::Task::Status RunAtomic() final;
        thread_pool::Task::Status SetUp();
        void CleanUp() final;

        /**
         * @brief Sets the ClientFD for this agent.
         * @param id
         * @note Needs to be set before being added to the thread pool.
         */
        void SetClientFD(int id);

        /**
         * @brief Sets the list of active commands for this agent.
         * @param cmds The list of active commands
         * @note Needs to be set before being added to the thread pool.
         */
        void SetActiveCommands(std::shared_ptr<CommandIDs> cmds);

        void SetFileAccessManagers(std::shared_ptr<server::file_handler::FileAccessManager> read_mgr,
                                   std::shared_ptr<server::file_handler::FileAccessManager> write_mgr);

    private:

        ///The agent.
        server::Agent* agent_;

        ///This AgentTask's client.
        int client_fd_;

        ///Active Commands @Note: To be inherited from the ServerTask, to be supplied to the agent
        std::shared_ptr<CommandIDs> active_commands_;


        ///The file access managers. @note To be inherited from NPortTask.
        std::shared_ptr<server::file_handler::FileAccessManager> read_manager_;
        std::shared_ptr<server::file_handler::FileAccessManager> write_manager_;

    };
}
#endif //PROJECT1_AGENT_TASK_H
