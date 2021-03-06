#include "file_access_manager.h"

#include <loguru.hpp>

namespace server::file_handler {
namespace {

using std::filesystem::path;

/**
 * @brief Converts a relative path to a normalized absolute one.
 * @param relative The relative path to convert.
 * @return The normalized absolute path.
 */
path UniquePath(const path& relative) {
  if (relative.is_absolute()) {
    // Nothing more needs to be done.
    return relative.lexically_normal();
  }

  // Make it absolute.
  const path kCurrentDir = std::filesystem::current_path();
  const path kAbsolute = kCurrentDir / relative;
  return kAbsolute.lexically_normal();
}

}  // namespace

void FileAccessManager::LockFile(const path& path) {
  const auto kPathKey = UniquePath(path).string();

  {
    std::unique_lock<std::mutex> lock(mutex_);

    // Make sure the file is not currently locked.
    if (locked_files_.find(kPathKey) != locked_files_.end()) {
      // Wait for file to be unlocked.
      file_unlocked_.wait(lock, [this, kPathKey]() {
        return locked_files_.find(kPathKey) == locked_files_.end();
      });
    }

    // Lock the file.
    LOG_S(1) << "Locking " << kPathKey << ".";
    locked_files_.insert(kPathKey);
  }
}

void FileAccessManager::UnlockFile(const path& path) {
  const auto kPathKey = UniquePath(path).string();

  {
    std::lock_guard<std::mutex> lock(mutex_);

    // Mark the file as unlocked.
    LOG_S(1) << "Unlocking " << kPathKey << ".";
    LOG_IF_S(WARNING, locked_files_.find(kPathKey) == locked_files_.end())
        << "Unlocking file that was never locked.";
    locked_files_.erase(kPathKey);
  }
  // Notify the next waiter.
  file_unlocked_.notify_one();
}

}  // namespace server::file_handler