#ifndef PROJECT1_FILE_ACCESS_MANAGER_H
#define PROJECT1_FILE_ACCESS_MANAGER_H

#include <filesystem>
#include <mutex>
#include <string>
#include <unordered_set>
#include <condition_variable>

namespace server::file_handler {

/**
 * @brief Facilitates synchronization among multiple threads that are
 *  using the file system.
 * @details This class is meant to be used with `FileHandlers`. Note that
 *  a single instance must be shared among threads for it to be effective.
 */
class FileAccessManager {
 public:
  /**
   * @brief Locks a particular file. When a file is locked, any calls to
   *    this method for the same file in other threads will block.
   * @param path The path to the file.
   */
  void LockFile(const std::filesystem::path& path);

  /**
   * @brief Unlocks a particular file. This will allow other threads to lock
   *    it.
   * @param path The path to the file.
   */
  void UnlockFile(const std::filesystem::path& path);

 private:
  /// Protects access to internal data structures.
  std::mutex mutex_{};
  /// This variable is notified when a file is unlocked.
  std::condition_variable file_unlocked_{};

  /// Set of absolute paths to files that are currently locked.
  std::unordered_set<std::string> locked_files_{};

};

}  // namespace server::file_handler

#endif  // PROJECT1_FILE_ACCESS_MANAGER_H
