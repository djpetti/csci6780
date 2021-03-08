/**
 * @file FTP Client Class
 */
#ifndef PROJECT1_CLIENT_H
#define PROJECT1_CLIENT_H

#include <cstdint>

#include "../server/file_handler/file_handler.h"
#include "../wire_protocol/wire_protocol.h"
#include "ftp_messages.pb.h"
#include "input_parser/input_parser.h"
#include "client_util.h"

namespace client {

/**
 * @brief FTP Client. Contains logic for connecting to server, sending requests,
 * outputting responses, and shell continuity.
 *
 */
class Client {
 public:
  /**
   * @brief The shell logic.
   *
   * @return True when user properly quits
   *
   * @note will never return false.
   */
  bool FtpShell();

  /**
   *
   * @param hostname the ip address of the FTP server
   * @param port the port the FTP server is binded to
   * @return true on success, false on failure
   */
  bool Connect(const std::string &hostname, uint16_t nport, uint16_t tport);

  /**
   * @brief sends the serialized message via a socket
   * @return true on success, false on failure
   */
  bool SendReq();

  /**
   * @brief expects a message from socket, stores message in parser
   * @return true on message received, false otherwise
   */
  bool WaitForMessage();

  /**
   * @brief extracts relevant information to be displayed to user from response
   */
  void HandleResponse();

 private:
  /**
   * @brief Handles output formatting for responses
   */
  void Output();

  // tracking connection status
  bool connected_;

  // buffer that stores serialized data to be sent to and received from server
  std::vector<uint8_t> outgoing_msg_buf_{};
  std::vector<uint8_t> incoming_msg_buf_{};

  // buffer size for client.
  static constexpr size_t kBufferSize = 4096;

  // hostname of the server
  std::string hostname_;

  // client port
  uint16_t nport_;

  // terminate port;
  uint16_t tport_;

  // client socket fd
  int client_fd_;

  // response info to be formatted and outputted
  std::string output_;

  // parser for handling messages
  wire_protocol::MessageParser<google::protobuf::Message> parser_;

  // parser for user input
  client::input_parser::InputParser *ip_;
};
}  // namespace client

#endif  // PROJECT1_CLIENT_H
