#ifndef PROJECT1_CHUNKED_FILE_SENDER_H
#define PROJECT1_CHUNKED_FILE_SENDER_H

#include <cstdint>
#include <string>
#include <vector>

namespace chunked_files {

/**
 * @brief Helper class for sending files in chunks.
 */
class ChunkedFileSender {
 public:
  /**
   * @param socket The socket to send data on.
   */
  explicit ChunkedFileSender(int socket);

  /**
   * @brief Sends the next chunk of the file. Call this repeatedly after
   *    calling SetFileContents.
   * @return Total number of bytes sent on the socket, 0 if the receiver
   *    disconnected, -1 for an error.
   */
  int SendNextChunk();

  /**
   * @return True if it sent the complete file.
   */
  [[nodiscard]] bool SentCompleteFile() const;

  /**
   * @brief Sets the complete contents of the file.
   * @param contents The file contents to send.
   */
  void SetFileContents(const std::vector<uint8_t> &contents);

 private:
  /// Buffer used for storing raw outgoing data.
  std::vector<uint8_t> outgoing_message_buffer_{};
  /// Internal buffer to store raw file data.
  std::vector<uint8_t> file_contents_{};

  /// Number of bytes from the file that we have sent so far.
  uint32_t total_bytes_sent_ = 0;

  /// Socket to send data on.
  int socket_;
};

}  // namespace chunked_files

#endif  // PROJECT1_CHUNKED_FILE_SENDER_H
