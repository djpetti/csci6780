/**
 * @file Implementation of common file handling operations
 */

#ifndef PROJECT1_FILE_HANDLER_H
#define PROJECT1_FILE_HANDLER_H

#include <sys/stat.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

#include "file_handler_interface.h"

/**
 * @brief This class represents common file handling operations.
 *
 */
namespace server::file_handler {
class FileHandler : public IFileHandler {
 public:
  FileHandler();

  [[nodiscard]] std::vector<uint8_t> Get(
      const std::string& filename) const override;

  bool Put(const std::string& filename,
           const std::vector<uint8_t>& contents) override;

  bool Delete(const std::string& filename) override;

  [[nodiscard]] std::vector<std::string> List() const final;

  bool ChangeDir(const std::string& sub_folder) final;

  bool UpDir() final;

  bool MakeDir(const std::string& name) override;

  [[nodiscard]] std::string GetCurrentDir() const final;

 private:
  /// Keeps track of the current directory.
  std::filesystem::path current_dir_;
};
}  // namespace server::file_handler
#endif  // PROJECT1_FILE_HANDLER_H
