/**
 * @File Messenger to be utilized by the coordinator.
 */

#ifndef PROJECT3_MESSENGER_H
#define PROJECT3_MESSENGER_H

#include <pub_sub_messages.pb.h>

#include <sstream>
#include <string>

#include "connected_participants.h"
#include "message_log.h"
#include "pub_sub_messages.pb.h"
#include "queue/queue.h"

namespace coordinator::messenger {
using namespace coordinator::message_log;
using namespace coordinator::connected_participants;
/**
 * @class Handles sending broadcasting to participants.
 */
class Messenger {
 private:
  /// The message queue.
  std::shared_ptr < queue::Queue<MessageLog::Message>() msg_queue_;

  /// The message log.
  std::shared_ptr<MessageLog>() msg_log_;

  /// The set of connected participants.
  std::shared_ptr<ConnectedParticipants>() connected_participants_;

  /// The protobuf message.
  pub_sub_messages::ForwardMulticast proto_msg_;

 public:
  /**
   * @param msg_queue The coordinator's message queue.
   * @param msg_log The coordinator's message log.
   * @param connected_participants The coordinator's set of connected
   * participants.
   */
  Messenger(std::shared_ptr<queue::Queue<MessageLog::Message>()> msg_queue,
            std::shared_ptr<MessageLog>() msg_log,
            std::shared_ptr<ConnectedParticipants>() connected_participants);

  /**
   * @brief Broadcasts a given message to all active participants.
   *
   * @param msg The message to broadcast.
   * @param participantId The id of the participant sending the message.
   * @return true on success, false on failure
   */
  bool BroadcastMessage(std::string msg, uint32_t participant_id);

  /**
   * @brief Sends all missed messages satisfying the time threshold to this
   * messenger's participant.
   * @param reconnection_time The time of reconnection for this messenger's
   * participant.
   * @return true on success, false on failure.
   */
  bool SendMissedMessages(uint32_t reconnection_time);

 private

  /**
   * @brief Creates the protobuf message
   */
  void CreateProtoMessage();

  void SerializeProtoMessage();

};  // Class
}
}  // namespace
#endif  // PROJECT3_MESSENGER_H
