/**
 * @File The thread safe data structure to store active command ID's
 */

#ifndef PROJECT1_COMMAND_IDS_H
#define PROJECT1_COMMAND_IDS_H

#include <mutex>
#include <algorithm>
#include <unordered_set>
namespace server_tasks {

    /**
     * @class Represents a thread-safe set of command IDs for active GET and PUT requests.
     */
    class CommandIDs {
    public:

        /**
         * @brief Inserts a command id into the set.
         * @param id The id to insert.
         */
        void Insert(uint16_t id);

        /**
         * @brief Determines if a certain ID is in the set.
         * @param id The id to check.
         * @return True if id is in the set, false otherwise
         */
        bool Contains(uint16_t id);

        /**
         * @brief Deletes a command id from the set.
         * @param id The id to delete.
         */
        void Delete(uint16_t id);

        /**
         * @brief Generates an id for a GET or PUT task.
         * @return the id
         */
        uint16_t GenerateID();

    private:

        /// The set of active command ids.
        std::unordered_set<uint16_t> command_ids_;

        /// Mutex for implementing thread safety.
        std::mutex mutex_;

        /// The id to give to new GET or PUT commands. @note Will increment if it's current value is already in use.
        uint16_t id_ = 0;
    };
}// namespace server_tasks
#endif //PROJECT1_COMMAND_IDS_H
