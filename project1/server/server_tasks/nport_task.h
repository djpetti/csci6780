/**
 * @File Task for monitoring port for 'Normal Commands'
 */

#ifndef PROJECT1_NPORT_TASK_H
#define PROJECT1_NPORT_TASK_H

#include "server_task.h"
#include "../../thread_pool/task.h"
#include "../../thread_pool/thread_pool.h"
#include "command_ids.h"
#include "../file_handler/file_handler.h"
#include "../file_handler/file_access_manager.h"


namespace server_tasks {

    /**
     * @class Monitors for client connections to the Normal Command Port.
     */
    class NPortTask : public ServerTask {
    public:

        /**
         * @brief Initializes all necessary information to run this task.
         * @param active_ids The set of active command IDs
         * @param port The port to bind to
         * @param read_mgr The read file access manager
         * @param write_mgr The write file access manager
         */
        NPortTask(std::shared_ptr<CommandIDs> active_ids, uint16_t port,
        std::shared_ptr<server::file_handler::FileAccessManager> read_mgr,
                  std::shared_ptr<server::file_handler::FileAccessManager> write_mgr)
                  : ServerTask(active_ids,port){};


        /**
         * @brief Listens for client socket connections.
         * @return Status::RUNNING
         */
        thread_pool::Task::Status Listen() override;


    private:

        ///The file access managers. @Note Inherited from the server.
        std::shared_ptr<server::file_handler::FileAccessManager> read_manager_;
        std::shared_ptr<server::file_handler::FileAccessManager> write_manager_;



    };
}
#endif //PROJECT1_NPORT_TASK_H
