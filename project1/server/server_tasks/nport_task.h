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

    class NPortTask : public ServerTask {
    public:
        thread_pool::Task::Status RunAtomic() final override;

        thread_pool::Task::Status SetUp() override;

        void CleanUp() final override;

        thread_pool::Task::Status Listen() override;

        void SetFileAccessManagers(std::shared_ptr<server::file_handler::FileAccessManager> read_mgr,
                                   std::shared_ptr<server::file_handler::FileAccessManager> write_mgr);

    private:

        ///The file access managers. @Note Inherited from the server.
        std::shared_ptr<server::file_handler::FileAccessManager> read_manager_;
        std::shared_ptr<server::file_handler::FileAccessManager> write_manager_;



    };
}
#endif //PROJECT1_NPORT_TASK_H
