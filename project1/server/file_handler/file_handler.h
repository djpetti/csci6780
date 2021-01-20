/**
 * @file Common interface for filesystem operations.
 */

#ifndef PROJECT1_FILE_HANDLER_H
#define PROJECT1_FILE_HANDLER_H

#include <cstdint>
#include <string>
#include <vector>

using namespace std;
namespace server {
namespace file_handler {

/**
 * @brief Common interface for filesystem operations.
 */
class IFileHandler {
  /**
   * @brief Gets the contents of a file in the current remote directory on the
   *    server.
   * @param filename The name of the file to get.
   * @return The binary contents of the file.
   */
  virtual vector<uint8_t> Get(const string& filename) const = 0;

  /**
   * @brief Creates a new file in the current remote directory on the server.
   * @param filename The name of the file to create.
   * @param contents The binary contents of the new file.
   * @return True on success, false on failure.
   */
  virtual bool Put(const string& filename,
                   const vector<uint8_t>& contents) = 0;

  /**
   * @brief Deletes a file in the current remote directory on the server.
   * @param filename The name of the file to delete.
   * @return True on success, false on failure.
   */
  virtual bool Delete(const string& filename) = 0;

  /**
   * @brief Lists the files in the current remote directory.
   * @return The list of files on the remote.
   */
  virtual std::vector<string> List() const = 0;

  /**
   * @brief Changes to a new directory that is a sub-folder of the current one.
   * @param sub_folder The sub-folder to enter.
   * @return True on success, false on failure.
   */
  virtual bool ChangeDir(const string& sub_folder) = 0;

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
  virtual bool MakeDir(const string& name) = 0;

  /**
   * @return The full path to the current remote directory.
   */
  virtual std::string GetCurrentDir() const = 0;
};

}  // namespace file_handler
}  // namespace server

#endif  // PROJECT1_FILE_HANDLER_H
