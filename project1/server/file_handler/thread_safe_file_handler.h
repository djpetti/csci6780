#ifndef PROJECT1_THREAD_SAFE_FILE_HANDLER_H
#define PROJECT1_THREAD_SAFE_FILE_HANDLER_H

#include <filesystem>
#include <memory>
#include <vector>

#include "file_access_manager.h"
#include "file_handler.h"

namespace server::file_handler {

/**
 * @brief A thread-safe implementation of the file handler interface. It is
 * suitable to have one instance per thread, and they will be synchronized
 * globally.
 */
class ThreadSafeFileHandler : public FileHandler {
 public:
  /**
   * @param read_manager Used for synchronizing access to files among multiple
   *    instances in different threads.
   */
  explicit ThreadSafeFileHandler(
      std::shared_ptr<FileAccessManager> read_manager,
      std::shared_ptr<FileAccessManager> write_manager);

  [[nodiscard]] std::vector<uint8_t> Get(
      const std::string &filename) const final;
  bool Put(const std::string &filename,
           const std::vector<uint8_t> &contents) final;
  bool Delete(const std::string &filename) final;
  bool MakeDir(const std::string &name) final;

 private:
  /**
   * @brief Computes an absolute path from this one based on the current
   * directory.
   * @param path The relative path to use.
   * @return The absolute path.
   */
  [[nodiscard]] std::filesystem::path ToAbsolute(
      const std::filesystem::path &path) const;

  /// Synchronizes read access to files from multiple threads.
  std::shared_ptr<FileAccessManager> read_manager_;
  /// Synchronizes write access to files from multiple threads.
  std::shared_ptr<FileAccessManager> write_manager_;
};

}  // namespace server::file_handler

#endif  // PROJECT1_THREAD_SAFE_FILE_HANDLER_H
