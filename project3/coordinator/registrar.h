/**
 * @file Registrar to be utilized by the coordinator.
 */
#ifndef PROJECT3_REGISTRAR_H
#define PROJECT3_REGISTRAR_H
#include "connected_participants.h"
#include <memory>

namespace coordinator::registrar {
using namespace connected_participants;
/**
 * @class Handles registration/de-registration of participants.
 */
class Registrar {
 private:

  /// The connected participants.
  std::shared_ptr<ConnectedParticipants> connected_participants_;

 public:
  /**
   * @param participants The coordinator's set of connected participants.
   */
  Registrar(std::shared_ptr<ConnectedParticipants> connected_participants);

  /**
   * @brief Registers a participant.
   * @param participant The participant to register
   */
  void RegisterParticipant(struct ConnectedParticipants::Participant &participant);

  /**
   * @brief De-registers a participant.
   * @param participant
   */
  void DeregisterParticipant(struct ConnectedParticipants::Participant &participant);

  /**
   * @brief Adds a participant to the connected participants list.
   * @param participant The participant to add.
   */
  void ConnectParticipant(struct ConnectedParticipants::Participant &participant);

  /**
   * @brief Removes a participant from the connected participants list.
   * @param participant The participant to remove.
   */
  void DisconnectParticipant(struct ConnectedParticipants::Participant &participant);

};  // Class
}  // namespace coordinator::registrar
#endif  // PROJECT3_REGISTRAR_H
