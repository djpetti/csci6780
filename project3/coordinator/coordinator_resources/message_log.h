/**
 * @file The coordinator's message log.
 */
#ifndef PROJECT3_MESSAGE_LOG_H
#define PROJECT3_MESSAGE_LOG_H

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_set>

namespace coordinator {

/**
 * @class A log of all messages broadcast by the coordinator.
 */
class MessageLog {
 public:
  /// Duration type to use for our timestamps.
  using Duration = std::chrono::steady_clock::duration;
  /// Type alias for timestamps.
  using Timestamp = std::chrono::steady_clock::time_point;

  /**
   * Structure of messages stored by the coordinator.
   */
  struct Message {
    bool operator==(const Message&) const;
    /// The actual message.
    std::string msg;

    /// The time the message was sent.
    Timestamp timestamp;

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
   * @param time_threshold Any messages older than this will be dropped
   *    when sending messages to reconnecting participants.
   */
  MessageLog(const Duration& time_threshold);

  /**
   * @brief Inserts a message into the message log.
   * @param msg The message to insert.
   */
  void Insert(const Message& msg);

  /**
   * @brief Clears the message log.
   */
  void Clear();

  /**
   * @brief Retrieves missed messages satisfying the reconnection time
   * threshold.
   * @param reconnection_time The time at which the participant reconnected.
   * @return All messages that were sent within the threshold before
   *    reconnection time.
   */
  std::unordered_set<Message, Hash> GetMissedMessages(
      const Timestamp& reconnection_time);

 private:
  /// The message log.
  std::unordered_set<Message, Hash> coordinator_messages_;

  /// Mutex for implementing thread safety.
  std::mutex mutex_;

  /// The reconnection time threshold for missed messages.
  Duration td_;

};  // class

}  // namespace coordinator

#endif  // PROJECT3_MESSAGE_LOG_H
