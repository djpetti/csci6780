/**
 * @file Implementation of MessengerManager class.
 */
#include "messenger_manager.h"

#include <loguru.hpp>
#include <utility>
namespace coordinator {
MessengerManager::MessengerManager(
    std::shared_ptr<ParticipantManager> participants)
    : participants_(std::move(participants)) {}

void MessengerManager::AddMessenger(
    const std::shared_ptr<Messenger>& messenger) {
  std::lock_guard<std::mutex> guard(mutex_);
  messengers_.insert(messenger);
}

void MessengerManager::DeleteMessenger(
    const std::shared_ptr<Messenger>& messenger) {
  std::lock_guard<std::mutex> guard(mutex_);
  messengers_.erase(messenger);
}

std::unordered_set<std::shared_ptr<Messenger>> MessengerManager::GetMessengers() {
  return messengers_;
}

bool MessengerManager::BroadcastMessage(MessageLog::Message *msg) {
  bool is_logged = false;
  for (auto& messenger : messengers_) {
    // only send message to connected participants.
    if (participants_->IsConnected(messenger->GetParticipant())) {
      messenger->SendMessage(*msg);
      LOG_F(INFO, "Sending message from Participant #%i to Participant #%i.",
            msg->participant_id, messenger->GetParticipant().id);
      // only log message once.
      // message will have timestamp of the first message sent.
      if (!is_logged) {
        messenger->LogMessage(msg);
        is_logged = true;
      }
    }
  }
  LOG_F(INFO, "Message from Participant #%i successfully broadcast.", msg->participant_id);
  return true;
}
}  // namespace coordinator
