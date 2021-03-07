/**
 * @File Task for monitoring port for 'Terminate' Commands
 */

#ifndef PROJECT1_TPORT_TASK_H
#define PROJECT1_TPORT_TASK_H

#include "server_task.h"
#include "../../thread_pool/task.h"
#include "../../thread_pool/thread_pool.h"
#include "command_ids.h"

namespace server_tasks {

    class TPortTask : public ServerTask {
    public:
        thread_pool::Task::Status RunAtomic() final override;

        thread_pool::Task::Status SetUp() override;

        void CleanUp() final override;

        thread_pool::Task::Status Listen() final override;


    };
}
#endif //PROJECT1_TPORT_TASK_H
