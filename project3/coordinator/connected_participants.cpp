/**
 * @file Implementation of ConnectedParticipants class.
 */
#include "connected_participants.h"

namespace coordinator {

void ConnectedParticipants::AddParticipant(const Participant &participant) {
  std::lock_guard<std::mutex> guard(mutex_);
  connected_participants_.insert(participant);
}

void ConnectedParticipants::DeleteParticipant(const Participant &participant) {
  std::lock_guard<std::mutex> guard(mutex_);
  connected_participants_.erase(participant);
}

std::unordered_set<ConnectedParticipants::Participant,
                   ConnectedParticipants::Hash>
ConnectedParticipants::GetParticipants() {
  std::lock_guard<std::mutex> guard(mutex_);
  return connected_participants_;
}
bool ConnectedParticipants::Participant::operator==(
    const Participant &participant) const {
  return id == participant.id;
}
size_t ConnectedParticipants::Hash::operator()(
    const Participant &participant) const {
  return std::hash<int>()(participant.id);
}

}  // namespace coordinator