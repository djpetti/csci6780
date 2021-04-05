/**
 * @File Messenger to be utilized by the coordinator.
 */
#ifndef PROJECT3_MESSENGER_H
#define PROJECT3_MESSENGER_H

#include <pub_sub_messages.pb.h>

#include <memory>
#include <sstream>
#include <string>

#include "connected_participants.h"
#include "message_log.h"
#include "pub_sub_messages.pb.h"
#include "queue/queue.h"
#include "wire_protocol/wire_protocol.h"

namespace coordinator {

/**
 * @class Handles message broadcasting to participants.
 *
 */
class Messenger {
 public:
  /**
   * @param msg_queue The coordinator's message queue.
   * @param msg_log The coordinator's message log.
   * @param participant The specific participant that this Messenger
   *    communicates with.
   */
  Messenger(std::shared_ptr<MessageLog> msg_log,
            ConnectedParticipants::Participant &participant);

  /**
   * @brief Sends a given message to this messenger's participant.
   * @param msg The message to send.
   * @return true on success, false on failure
   */
  bool SendMessage(const MessageLog::Message msg);

  /**
   * @brief Sends all missed messages satisfying the time threshold to this
   * messenger's participant.
   * @param reconnection_time The time of reconnection for this messenger's
   * participant.
   * @return true on success, false on failure.
   */
  bool SendMissedMessages(const MessageLog::Timestamp& reconnection_time);

  /**
   * @brief Updates message timestamp and registers message with log.
   * @param msg The message.
   */
  void LogMessage(MessageLog::Message *msg);

  /**
   * @brief Getter for this messenger's participant.
   * @return This messenger's participant.
   */
  const ConnectedParticipants::Participant& GetParticipant() const;

 private:

  /// The message log.
  std::shared_ptr<MessageLog> msg_log_;

  /// The participant using this messenger.
  ConnectedParticipants::Participant participant_;

  /// Mutex for implementing thread safety.
  std::mutex mutex_;

  /// Protobuf message.
  pub_sub_messages::ForwardMulticast proto_msg_;

  /// Buffer for outgoing messages.
  std::vector<uint8_t> outgoing_message_buffer_{};

  /**
   * Helper function for Serializing Messages.
   */
  bool SerializeMessage(MessageLog::Message msg);

};  // Class

}  // namespace coordinator

#endif  // PROJECT3_MESSENGER_H
