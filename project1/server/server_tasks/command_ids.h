/**
 * @File The thread safe data structure to store active command ID's
 */

#ifndef PROJECT1_COMMAND_IDS_H
#define PROJECT1_COMMAND_IDS_H

#include <mutex>
#include <algorithm>
#include <list>
namespace server_tasks {
    class CommandIDs {
    public:

        void Insert(uint16_t id);
        bool Contains(uint16_t id);
        void Delete(uint16_t id);

    private:
        std::list<uint16_t> command_ids_;
        std::mutex mutex_;
    };
}// namespace server_tasks
#endif //PROJECT1_COMMAND_IDS_H
