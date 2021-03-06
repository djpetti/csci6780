#include "file_lock_guard.h"

#include <utility>

namespace server::file_handler {

FileLockGuard::FileLockGuard(FileAccessManager *manager,
                             std::filesystem::path path)
    : manager_(manager), path_(std::move(path)) {
  // Lock the file.
  manager_->LockFile(path_);
}

FileLockGuard::~FileLockGuard() {
  // Unlock the file.
  manager_->UnlockFile(path_);
}

}  // namespace server::file_handler
