/**
 * Handles participant commands.
 */

#ifndef CSCI6780_COORDINATOR_H
#define CSCI6780_COORDINATOR_H
#include "messenger.h"
#include "messenger_manager.h"
#include "registrar.h"

namespace coordinator {
class Coordinator {
 public:
  Coordinator(int participant_fd, std::string hostname,
                           std::shared_ptr<MessengerManager> msg_mgr,
                           std::shared_ptr<Registrar> registrar,
              std::shared_ptr<queue::Queue<MessageLog::Message>> msg_queue,
              std::shared_ptr<MessageLog> msg_log);

  bool Coordinate();

 private:

  /// The messenger
  std::shared_ptr<Messenger> messenger_;
  /// The message queue
  std::shared_ptr<queue::Queue<MessageLog::Message>> msg_queue_;
  /// The message log
  std::shared_ptr<MessageLog> msg_log_;
  /// The participant being coordinated.
  ConnectedParticipants::Participant participant_;
  /// Enumerates state of connected client.
  enum class ClientState {
    /// We expect more messages from the client.
    ACTIVE,
    /// The client has disconnected normally.
    DISCONNECTED,
    /// There was some error communicating with the client.
    ERROR,
  };
  /// Size in bytes to use for the internal message buffer.
  static constexpr size_t kClientBufferSize = 1000;

  /// The client socket fd.
  int client_fd_;

  /// The hostname of the client.
  std::string hostname_;

  /// The messenger manager.
  std::shared_ptr<MessengerManager> msg_mgr_;

  /// The registrar.
  std::shared_ptr<Registrar> registrar_;

  /// Parser to use for reading messages on the socket.
  wire_protocol::MessageParser<pub_sub_messages::CoordinatorMessage> parser_;

  /// Internal buffer to use for incoming messages.
  std::vector<uint8_t> incoming_message_buffer_{};

  /// ID counter.
  static uint32_t id_;

  /**
   * @brief Reads the next message from the socket.
   * @param message The message to read into.
   * @return True if it succeeded in reading the message, false otherwise.
   */
  ClientState ReadNextMessage(pub_sub_messages::CoordinatorMessage *message);

  ClientState DispatchMessage(const pub_sub_messages::CoordinatorMessage &message);

  ClientState HandleRequest(const pub_sub_messages::Register &request);

  ClientState HandleRequest(const pub_sub_messages::Deregister &request);

  ClientState HandleRequest(const pub_sub_messages::Disconnect &request);

  ClientState HandleRequest(const pub_sub_messages::Reconnect &request);

  ClientState HandleRequest(const pub_sub_messages::SendMulticast &request);


  uint32_t GenerateID();
};
}  // namespace coordinator
#endif  // CSCI6780_COORDINATOR_H
