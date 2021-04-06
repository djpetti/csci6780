/**
 * @file The connected Participants.
 */
#ifndef PROJECT3_CONNECTED_PARTICIPANTS_H
#define PROJECT3_CONNECTED_PARTICIPANTS_H

#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace coordinator {

/**
 * @class Manages participants known to the system.
 */
class ParticipantManager {
 public:
  /**
   * The structure for Participants
   */
  struct Participant {
    bool operator==(const Participant&) const;
    /// The id.
    uint32_t id;

    /// The socket file descriptor for this participant's message listening
    /// thread.
    int sock_fd;
    /// The port number this participant receives messages on.
    uint16_t port;
    /// The machine name this participant receives messages on.
    std::string hostname;

    /// True if the participant is currently connected.
    bool connected;
  };

  /**
   * Structure for hash override for unordered_set of Participants
   */
  struct Hash {
    size_t operator()(const Participant& participant) const;
  };

  /// Alias for a set that stores participants.
  using ParticipantSet = std::unordered_set<Participant, Hash>;

  /**
   * @brief Adds a participant to the set of registered participants.
   * @param participant The participant to add.
   */
  void RegisterParticipant(const Participant& participant);

  /**
   * @brief Deletes a participant from the set of registered participants.
   * @param participant The participant to delete.
   */
  void DeregisterParticipant(const Participant& participant);

  /**
   * @brief Marks a participant as disconnected. It will still be tracked
   * though.
   * @param participant The participant to disconnect.
   * @return True if the participant was successfully disconnected, false
   *    if it was not connected to begin with.
   */
  bool DisconnectParticipant(const Participant& participant);

  /**
   * @brief Marks a participant as reconnected.
   * @param participant The participant to mark as reconnected.
   * @return True if the participant was successfully found and modified, false
   *    if there was no disconnected participant with that ID.
   */
  bool ReconnectParticipant(const Participant& participant);

  /**
   * @brief Gets a participant by its ID.
   * @param id The participant ID.
   * @param participant[out] Will be set to the Participant.
   * @return True on success, false if there is no connected participant with
   *    that ID.
   */
  bool GetParticipant(uint32_t id, Participant* participant);

  /**
   * @brief Retrieves the set of currently connected participants.
   * @note Will return a copy of the set instead of a reference, in order
   *    to preserve thread-safety.
   * @return A copy of the set of connected participants.
   */
  ParticipantSet GetAllConnected();

  /**
   * Determines if a particular participant is connected.
   * @param participant The participant.
   * @return true if contains, false otherwise.
   */
  bool IsConnected(const Participant& participant);

 private:
  /// The connected participants
  ParticipantSet connected_participants_;
  /// All participants, indexed by ID.
  std::unordered_map<uint32_t, Participant> id_to_participant_;

  /// Mutex for implementing thread safety.
  std::mutex mutex_;
};  // class

}  // namespace coordinator

#endif  // PROJECT3_CONNECTED_PARTICIPANTS_H
