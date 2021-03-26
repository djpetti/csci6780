#ifndef PROJECT1_FILE_LOCK_GUARD_H
#define PROJECT1_FILE_LOCK_GUARD_H

#include <filesystem>

#include "file_access_manager.h"

namespace server::file_handler {

/**
 * @brief Trivial class that acts as the equivalent of `std::lock_guard` but
 *  with `FileAccessManagers`.
 */
class FileLockGuard {
 public:
  /**
   * @param manager The `FileAccessManager` to use for locking.
   * @param path The path to the file to lock.
   */
  FileLockGuard(FileAccessManager* manager, std::filesystem::path path);

  ~FileLockGuard();

 private:
  /// Internal FileAccessManager to use for locking.
  FileAccessManager* manager_;
  /// Path that we have locked.
  std::filesystem::path path_;
};

}  // namespace server::file_handler

#endif  // PROJECT1_FILE_LOCK_GUARD_H
