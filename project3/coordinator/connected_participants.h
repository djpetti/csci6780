/**
 * @file The connected Participants.
 */
#ifndef PROJECT3_CONNECTED_PARTICIPANTS_H
#define PROJECT3_CONNECTED_PARTICIPANTS_H

#include <algorithm>
#include <mutex>
#include <unordered_set>

namespace coordinator {

/**
 * @class The connected Participants with thread-safety.
 */
class ConnectedParticipants {
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
  };

  /**
   * Structure for hash override for unordered_set of Participants
   */
  struct Hash {
    size_t operator()(const Participant& participant) const;
  };

  /**
   * @brief Adds a participant to the set of connected participants.
   * @param participant The participant to add.
   */
  void AddParticipant(const Participant& participant);

  /**
   * @brief Deletes a participant from the set of connected participants.
   * @param participant
   */
  void DeleteParticipant(const Participant& participant);

  /**
   * @brief Retrieves the set of currently connected participants.
   * @note Will return a copy of the set instead of a reference, in order
   *    to preserve thread-safety.
   * @return A copy of the set of connected participants.
   */
  std::unordered_set<Participant, Hash> GetParticipants();

 private:
  /// The connected participants
  std::unordered_set<Participant, Hash> connected_participants_;

  /// Mutex for implementing thread safety.
  std::mutex mutex_;
};  // class

}  // namespace coordinator

#endif  // PROJECT3_CONNECTED_PARTICIPANTS_H
