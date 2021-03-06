/**
 * @file Common interface for filesystem operations.
 */

#ifndef PROJECT1_FILE_HANDLER_INTERFACE_H
#define PROJECT1_FILE_HANDLER_INTERFACE_H

#include <cstdint>
#include <string>
#include <vector>

namespace server::file_handler {

/**
 * @brief Common interface for filesystem operations.
 */
class IFileHandler {
 public:
  virtual ~IFileHandler() = default;

  /**
   * @brief Gets the contents of a file in the current remote directory on the
   *    server.
   * @param filename The name of the file to get.
   * @return The binary contents of the file.
   */
  [[nodiscard]] virtual std::vector<uint8_t> Get(
      const std::string& filename) const = 0;

  /**
   * @brief Creates a new file in the current remote directory on the server.
   * @param filename The name of the file to create.
   * @param contents The binary contents of the new file.
   * @return True on success, false on failure.
   */
  virtual bool Put(const std::string& filename,
                   const std::vector<uint8_t>& contents) = 0;

  /**
   * @brief Deletes a file in the current remote directory on the server.
   * @param filename The name of the file to delete.
   * @return True on success, false on failure.
   */
  virtual bool Delete(const std::string& filename) = 0;

  /**
   * @brief Lists the files in the current remote directory.
   * @return The list of files on the remote.
   */
  [[nodiscard]] virtual std::vector<std::string> List() const = 0;

  /**
   * @brief Changes to a new directory that is a sub-folder of the current one.
   * @param sub_folder The sub-folder to enter.
   * @return True on success, false on failure.
   */
  virtual bool ChangeDir(const std::string& sub_folder) = 0;

  /**
   * @brief Moves to the parent of the current directory.
   * @return True on success, false on failure.
   */
  virtual bool UpDir() = 0;

  /**
   * @brief Creates a new sub-directory within the current remote directory.
   * @param name The name of the sub-directory.
   * @return True on success, false on failure.
   */
  virtual bool MakeDir(const std::string& name) = 0;

  /**
   * @return The full path to the current remote directory.
   */
  [[nodiscard]] virtual std::string GetCurrentDir() const = 0;
};

}  // namespace server::file_handler

#endif  // PROJECT1_FILE_HANDLER_INTERFACE_H
