/**
 * @file The coordinator's message log.
 */
#ifndef PROJECT3_MESSAGE_LOG_H
#define PROJECT3_MESSAGE_LOG_H
#include <algorithm>
#include <mutex>
#include <unordered_set>
#include <memory>
namespace coordinator::message_log {

/**
 * @class A log of all messages broadcast by the coordinator.
 */
class MessageLog {
 public:
  /**
   * Structure of messages stored by the coordinator.
   */
  struct Message {
    bool operator==(const Message&) const;
    /// The actual message.
    std::string msg;

    /// The time the message was sent.
    double timestamp;

    /// TODO The participant who broadcast the message.
    uint32_t participant_id;
  };

  /**
   * Structure for hash function.
   */
  struct Hash {
    size_t operator()(const Message& obj) const;
  };
  /**
   * @brief Inserts a message into the message log.
   * @param id The id to insert.
   */
  void Insert(struct Message& msg);

  /**
   * @brief Clears the message log.
   */
  void Clear();

  /**
   * @brief Retrieves missed messages satisfying the reconnection time
   * threshold.
   * @param reconnection_time
   * @return
   */
  std::unordered_set<Message, Hash> GetMissedMessages(
      uint32_t reconnection_time);

 private:
  /// The message log.
  std::unordered_set<Message, Hash> coordinator_messages_;

  /// Mutex for implementing thread safety.
  std::mutex mutex_;

  /// The reconnection time threshold for missed messages.
  uint8_t td_;

};  // class
}  // namespace coordinator::message_log
#endif  // PROJECT3_MESSAGE_LOG_H
