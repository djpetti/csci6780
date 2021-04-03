/**
 * @File Messenger to be utilized by the coordinator.
 */
#ifndef PROJECT3_MESSENGER_H
#define PROJECT3_MESSENGER_H

#include <pub_sub_messages.pb.h>
#include <sstream>
#include <string>
#include <memory>

#include "connected_participants.h"
#include "message_log.h"
#include "pub_sub_messages.pb.h"
#include "queue/queue.h"
#include "wire_protocol/wire_protocol.h"
namespace coordinator::messenger {
using namespace coordinator::message_log;
using namespace coordinator::connected_participants;
/**
 * @class Handles message broadcasting to participants.
 *
 */
class Messenger {
 public:
  /**
   * @param msg_queue The coordinator's message queue.
   * @param msg_log The coordinator's message log.
   * @param connected_participants The coordinator's set of connected
   * participants.
   */
  Messenger(std::shared_ptr<queue::Queue<MessageLog::Message>> msg_queue,
            std::shared_ptr<MessageLog> msg_log,
            std::shared_ptr<ConnectedParticipants::Participant> participant,
            std::shared_ptr<ConnectedParticipants> connected_participants);

  /**
   * @brief Broadcasts a given message to all active participants.
   * @note Messages sent by the messenger should be pushed in the message queue before
   *       this function is called. Guarantees that messages will be broadcast in the order
   *       they were received.
   * @return true on success, false on failure
   *
   */
  bool BroadcastMessage();

  /**
   * @brief Sends all missed messages satisfying the time threshold to this
   * messenger's participant.
   * @param reconnection_time The time of reconnection for this messenger's
   * participant.
   * @return true on success, false on failure.
   */
  bool SendMissedMessages(uint32_t reconnection_time);

 private:
  /// The outgoing message queue.
  std::shared_ptr < queue::Queue<MessageLog::Message>> msg_queue_;

  /// The message log.
  std::shared_ptr<MessageLog> msg_log_;

  /// The set of connected participants.
  std::shared_ptr<ConnectedParticipants> connected_participants_;

  /// Internal buffer to use for outgoing messages.
  std::vector<uint8_t> outgoing_message_buffer_{};

  /// The protobuf message.
  pub_sub_messages::ForwardMulticast proto_msg_;

  /// The participant using this messenger.
  std::shared_ptr<ConnectedParticipants::Participant> participant_;

};  // Class
} // namespace
#endif  // PROJECT3_MESSENGER_H
