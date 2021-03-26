#include "thread_safe_file_handler.h"

#include <utility>

#include "file_lock_guard.h"

namespace server::file_handler {

ThreadSafeFileHandler::ThreadSafeFileHandler(
    std::shared_ptr<FileAccessManager> read_manager,
    std::shared_ptr<FileAccessManager> write_manager)
    : read_manager_(std::move(read_manager)),
      write_manager_(std::move(write_manager)) {}

std::vector<uint8_t> ThreadSafeFileHandler::Get(
    const std::string& filename) const {
  FileLockGuard read_lock(read_manager_.get(), ToAbsolute(filename));

  return FileHandler::Get(filename);
}

bool ThreadSafeFileHandler::Put(const std::string& filename,
                                const std::vector<uint8_t>& contents) {
  const auto kPath = ToAbsolute(filename);
  FileLockGuard read_lock(read_manager_.get(), kPath);
  FileLockGuard write_lock(write_manager_.get(), kPath);

  return FileHandler::Put(filename, contents);
}

bool ThreadSafeFileHandler::Delete(const std::string& filename) {
  const auto kPath = ToAbsolute(filename);
  FileLockGuard read_lock(read_manager_.get(), kPath);
  FileLockGuard write_lock(write_manager_.get(), kPath);

  return FileHandler::Delete(filename);
}

bool ThreadSafeFileHandler::MakeDir(const std::string& name) {
  const auto kPath = ToAbsolute(name);
  FileLockGuard read_lock(read_manager_.get(), kPath);
  FileLockGuard write_lock(write_manager_.get(), kPath);

  return FileHandler::MakeDir(name);
}

std::filesystem::path ThreadSafeFileHandler::ToAbsolute(
    const std::filesystem::path& path) const {
  return std::filesystem::path(GetCurrentDir()) / path;
}

}  // namespace server::file_handler