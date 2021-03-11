#ifndef PROJECT1_CHUNKED_FILE_RECEIVER_H
#define PROJECT1_CHUNKED_FILE_RECEIVER_H

#include <cstdint>
#include <string>
#include <vector>

#include "../wire_protocol/wire_protocol.h"
#include "ftp_messages.pb.h"

namespace chunked_files {

/**
 * @brief Helper class for receiving files in chunks.
 */
class ChunkedFileReceiver {
 public:
  /**
   * @param socket The socket to receive data on.
   */
  explicit ChunkedFileReceiver(int socket);

  /**
   * @brief Receives the next chunk of the file.
   * @return Total number of bytes read from the socket, 0 if the sender
   *    disconnected, -1 for an error.
   */
   int ReceiveNextChunk();

   /**
    * @return True if it read the complete file.
    */
   [[nodiscard]] bool HasCompleteFile() const;

   /**
    * @brief Gets the complete contents of the file.
    * @param contents The contents will be written here.
    */
   void GetFileContents(std::vector<uint8_t> *contents);

   /**
    * @brief Resets the state of the parser.
    */
   void Reset();

   /**
    * @brief Reads any remaining data from the socket until a complete message
    *   is parsed. This is mostly so we don't leave the socket in an
    *   indeterminate state.
    * @return False if it encountered a socket error.
    */
   bool CleanUp();

 private:
  /// Parser to use for parsing file data.
  wire_protocol::MessageParser<ftp_messages::FileContents> parser_;

  /// Buffer used for storing raw data from the socket.
  std::vector<uint8_t> incoming_message_buffer_{};
  /// Internal buffer to store parsed file data.
  std::string file_contents_{};

  /// Whether we have a complete file parsed.
  bool complete_file_ = false;

  /// Socket to receive data on.
  int socket_;
};

}  // namespace chunked_files

#endif  // PROJECT1_CHUNKED_FILE_RECEIVER_H
