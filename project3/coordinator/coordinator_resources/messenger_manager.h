/**
 * @file Manages all active messengers.
 */
#ifndef CSCI6780_MESSENGER_MANAGER_H
#define CSCI6780_MESSENGER_MANAGER_H
#include "messenger.h"
namespace coordinator {
class MessengerManager {
 public:

  /**
   * @param participants The connected participants.
   */
  explicit MessengerManager(std::shared_ptr<ParticipantManager> participants);
  /**
   * @brief Broadcasts a given message to all active participants.
   * @note Messages sent by the messenger should be pushed in the message queue before
   *       this function is called. Guarantees that messages will be broadcast in the order
   *       they were received.
   * @return true on success, false on failure
   *
   */
  bool BroadcastMessage(MessageLog::Message *message);

  /**
   * @brief Registers a messenger with the messenger manager.
   * @param messenger The messenger to register.
   */
  void AddMessenger(const std::shared_ptr<Messenger>& messenger);

  /**
   * @brief De-registers a messenger from the messenger manager.
   * @param messenger
   */
  void DeleteMessenger(const std::shared_ptr<Messenger>& messenger);

  /**
   * @brief Getter for the set of Messengers.
   * @return The set of messengers.
   * @note for read only.
   */
  std::unordered_set<std::shared_ptr<Messenger>> GetMessengers();
 private:

  /// The Messengers
  std::unordered_set<std::shared_ptr<Messenger>> messengers_;

  /// Mutex for implementing thread safety.
  std::mutex mutex_;

  /// The connected participants.
  std::shared_ptr<ParticipantManager> participants_;

};  // class
}  // namespace coordinator
#endif  // CSCI6780_MESSENGER_MANAGER_H
