/**
 * @file Implementation of MessengerManager class.
 */
#include "messenger_manager.h"
namespace coordinator {
MessengerManager::MessengerManager(
    std::shared_ptr<ConnectedParticipants> participants)
    : participants_(participants) {}

void MessengerManager::AddMessenger(const std::shared_ptr<Messenger> messenger) {
  std::lock_guard<std::mutex> guard(mutex_);
  messengers_.insert(messenger);
}

void MessengerManager::DeleteMessenger(const std::shared_ptr<Messenger> messenger) {
  std::lock_guard<std::mutex> guard(mutex_);
  messengers_.erase(messenger);
}

bool MessengerManager::BroadcastMessage(MessageLog::Message& msg) {
  bool is_logged = false;
  for (auto messenger : messengers_) {
    // only send message to connected participants.
    if (participants_->Contains(messenger->GetParticipant())){
      messenger->SendMessage(msg);

      //make sure to only log the message once.
      if (!is_logged) {
        messenger->LogMessage(msg);
        is_logged = true;
      }
    }
  }
  return true;
}
}  // namespace coordinator
