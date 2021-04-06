/**
 * @file Registrar to be utilized by the coordinator.
 */
#ifndef PROJECT3_REGISTRAR_H
#define PROJECT3_REGISTRAR_H

#include <memory>

#include "participant_manager.h"

namespace coordinator {

/**
 * @class Handles registration/de-registration of participants.
 */
class Registrar {
 public:
  /**
   * @param participants The coordinator's set of connected participants.
   */
  explicit Registrar(
      std::shared_ptr<ParticipantManager> participants);

  /**
   * @brief Registers a participant.
   * @param participant The participant to register
   */
  void RegisterParticipant(ParticipantManager::Participant *participant);

  /**
   * @brief De-registers a participant.
   * @param participant
   */
  void DeregisterParticipant(
      struct ParticipantManager::Participant &participant);

  /**
   * @brief Adds a participant to the connected participants list.
   * @param participant The participant to add.
   */
  void ReconnectParticipant(ParticipantManager::Participant *participant);

  /**
   * @brief Removes a participant from the connected participants list.
   * @param participant The participant to remove.
   */
  void DisconnectParticipant(
      const ParticipantManager::Participant &participant);

  /**
   * @brief Returns the set of connected participants.
   * @return The set of participants.
   * @note Used for read-only.
   */
  ParticipantManager *GetParticipants();

 private:
  /// The connected participants.
  std::shared_ptr<ParticipantManager> participants_;

};  // Class

}  // namespace coordinator

#endif  // PROJECT3_REGISTRAR_H
