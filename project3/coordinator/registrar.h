/**
 * @file Registrar to be utilized by the coordinator.
 */
#ifndef PROJECT3_REGISTRAR_H
#define PROJECT3_REGISTRAR_H
#include "connected_participants.h"

namespace coordinator::registrar {
using namespace connected_participants;
/**
 * @class Handles registration/de-registration of participants.
 */
class Registrar {
 public:
  /**
   * @param participants The coordinator's set of connected participants.
   */
  Registrar(std::shared_ptr<ConnectedParticipants>() participants);
  /**
   * @brief Registers a participant.
   * @param participant The participant to register
   */
  void RegisterParticipant(struct Participant &participant);

  /**
   * @brief De-registers a participant.
   * @param participant
   */
  void DeregisterParticipant(struct Participant &participant);

  /**
   * @brief Adds a participant to the connected participants list.
   * @param participant The participant to add.
   */
  void ConnectParticipant(struct Participant &participant);

  /**
   * @brief Removes a participant from the connected participants list.
   * @param participant The participant to remove.
   */
  void DisconnectParticipant(struct Participant &participant);

 private:
  /**
   * @brief Connects to the Participant's
   * @param hostname
   * @param port
   * @return
   */
  bool Connect(const std::string &hostname, uint16_t port);

  /// The connected participants.
  std::shared_ptr<ConnectedParticipants>() participants_;

  /// TODO Socket Connection elements.

};  // Class
}  // namespace coordinator::registrar
#endif  // PROJECT3_REGISTRAR_H
