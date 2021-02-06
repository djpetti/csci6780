/**
 * @file Logic for handling individual connections to the server.
 */

#ifndef PROJECT1_AGENT_H
#define PROJECT1_AGENT_H

#include <cstdint>
#include <memory>
#include <vector>

#include "../wire_protocol/wire_protocol.h"
#include "file_handler/file_handler_interface.h"
#include "ftp_messages.pb.h"

namespace server {

/**
 * @brief Contains logic for handling individual connections to the server.
 */
class Agent {
 public:
  /**
   * @param client_fd The FD of the client socket. Note that it will take
   *    responsibility for closing this socket on exit.
   * @param file_handler The `FileHandler` to use internally. Note this class
   *    will take ownership of it.
   */
  Agent(int client_fd,
        std::unique_ptr<file_handler::IFileHandler> file_handler);

  /**
   * @brief Handles this particular client, until the client disconnects.
   * @note This will block and is meant to be run in its own thread..
   * @return True if the client was serviced and disconnected normally, false
   *    if a failure forced a premature disconnect.
   */
  bool Handle();

 private:
  /// Size in bytes to use for the internal message buffer.
  static constexpr size_t kClientBufferSize = 4096;

  /// Enumerates state of connected client.
  enum class ClientState {
    /// We expect more messages from the client.
    ACTIVE,
    /// The client has disconnected normally.
    DISCONNECTED,
    /// There was some error communicating with the client.
    ERROR,
  };

  /**
   * @brief Reads the next message from the socket.
   * @param message The message to read into.
   * @return True if it succeeded in reading the message, false otherwise.
   */
  ClientState ReadNextMessage(ftp_messages::Request* message);

  /**
   * @brief Dispatches an incoming request to the proper handler.
   * @param message The request.
   * @return The updated state of the client.
   */
  ClientState DispatchMessage(const ftp_messages::Request& message);

  /**
   * @brief All these methods are handlers for specific types of requests.
   * @param request The request to handle.
   * @return The updated state of the client.
   */
  ClientState HandleRequest(const ftp_messages::GetRequest& request);
  ClientState HandleRequest(const ftp_messages::PutRequest& request);
  ClientState HandleRequest(const ftp_messages::DeleteRequest& request);
  ClientState HandleRequest(const ftp_messages::ListRequest& request);
  ClientState HandleRequest(const ftp_messages::ChangeDirRequest& request);
  ClientState HandleRequest(const ftp_messages::MakeDirRequest& request);
  ClientState HandleRequest(const ftp_messages::PwdRequest& request);
  ClientState HandleRequest(const ftp_messages::QuitRequest& request);

  /**
   * @brief Sends a response message to the client.
   * @param response The message to send.
   * @return The updated state of the client.
   */
  bool SendResponse(const ftp_messages::Response& response);

  /// The FD to talk to the client on.
  int client_fd_;

  /// Internal buffer to use for incoming messages.
  std::vector<uint8_t> incoming_message_buffer_{};
  /// Internal buffer to use for outgoing messages.
  std::vector<uint8_t> outgoing_message_buffer_{};
  /// Parser to use for reading messages on the socket.
  ::wire_protocol::MessageParser<ftp_messages::Request> parser_;

  /// Used for performing filesystem operations.
  std::unique_ptr<file_handler::IFileHandler> file_handler_;
};

}  // namespace server

#endif  // PROJECT1_AGENT_H
