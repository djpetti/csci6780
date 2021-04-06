/**
 * @file Implementation of ParticipantManager class.
 */
#include "participant_manager.h"

#include <chrono>

#include <loguru.hpp>
namespace coordinator {

void ParticipantManager::RegisterParticipant(const Participant &participant) {
  std::lock_guard<std::mutex> guard(mutex_);
  id_to_participant_[participant.id] = participant;
  connected_participants_.insert(participant);
}

void ParticipantManager::DeregisterParticipant(const Participant &participant) {
  std::lock_guard<std::mutex> guard(mutex_);
  id_to_participant_.erase(participant.id);
  connected_participants_.erase(participant);
}
bool ParticipantManager::IsConnected(const Participant &participant) {
  std::lock_guard<std::mutex> guard(mutex_);
  const auto &find = connected_participants_.find(participant);
  return find != connected_participants_.end();
}

std::unordered_set<ParticipantManager::Participant, ParticipantManager::Hash>
ParticipantManager::GetAllConnected() {
  std::lock_guard<std::mutex> guard(mutex_);
  return connected_participants_;
}

bool ParticipantManager::DisconnectParticipant(
    const ParticipantManager::Participant &participant) {
  std::lock_guard<std::mutex> guard(mutex_);

  // Remove this from the set of connected participants.
  const auto &participant_iter = connected_participants_.find(participant);
  if (participant_iter == connected_participants_.end()) {
    // No such participant.
    LOG_S(WARNING) << "Participant " << participant.id
                   << " is already not connected.";
    return false;
  }

  // Mark participant as disconnected.
  id_to_participant_[participant.id].connected = false;
  // Modify the disconnect time.
  id_to_participant_[participant.id].disconnect_time =
      std::chrono::steady_clock::now();
  // Remove from connected set.
  connected_participants_.erase(participant_iter);

  return true;
}

bool ParticipantManager::ReconnectParticipant(const Participant &participant) {
  std::lock_guard<std::mutex> guard(mutex_);

  auto participant_iter = id_to_participant_.find(participant.id);
  if (participant_iter == id_to_participant_.end()) {
    // No such participant.
    LOG_S(ERROR) << "Attempt to reconnect participant " << participant.id
                 << ", but no such participant exists.";
    return false;
  }

  // Mark that it is now connected.
  participant_iter->second.connected = true;
  // Mark as connected.
  connected_participants_.insert(participant_iter->second);

  return true;
}

bool ParticipantManager::GetParticipant(
    uint32_t id, ParticipantManager::Participant *participant) {
  std::lock_guard<std::mutex> guard(mutex_);

  const auto &kParticipantIter = id_to_participant_.find(id);
  if (kParticipantIter == id_to_participant_.end()) {
    // No such participant.
    LOG_S(ERROR) << "Attempt to get participant " << id
                 << " which does not exist.";
    return false;
  }

  *participant = kParticipantIter->second;

  return true;
}

bool ParticipantManager::Participant::operator==(
    const Participant &participant) const {
  return id == participant.id;
}
size_t ParticipantManager::Hash::operator()(
    const Participant &participant) const {
  return std::hash<int>()(participant.id);
}

}  // namespace coordinator