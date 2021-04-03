/**
 * @file Implementation of ConnectedParticipants class.
 */
#include "connected_participants.h"
namespace coordinator::connected_participants {

void ConnectedParticipants::AddParticipant(struct Participant &participant) {
  std::lock_guard<std::mutex> guard(mutex_);
  connected_participants_.insert(participant);
}

void ConnectedParticipants::DeleteParticipant(struct Participant &participant) {
  std::lock_guard<std::mutex> guard(mutex_);
  connected_participants_.erase(participant);
}

std::unordered_set<ConnectedParticipants::Participant,
                   ConnectedParticipants::Hash>
ConnectedParticipants::GetParticipants() {
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

}  // namespace coordinator::connected_participants