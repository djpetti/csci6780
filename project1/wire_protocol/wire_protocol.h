/**
 * @file Contains utilities for dealing with data sent over the network.
 */

#ifndef PROJECT1_WIRE_PROTOCOL_H
#define PROJECT1_WIRE_PROTOCOL_H

#include <arpa/inet.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

#include "google/protobuf/message.h"
#include "proto/ftp_messages.pb.h"

namespace wire_protocol {

/**
 * @brief Serializes a message to the wire format. Once this is done, it can
 *  be safely sent over a socket.
 * @param message The message to serialize.
 * @param[out] serialized Will be set to the serialized output data.
 * @return True if it succeeded in serializing, false otherwise.
 */
bool Serialize(const google::protobuf::Message& message,
               std::vector<uint8_t>* serialized);

/**
 * @brief A parser for serialized messages.
 * @tparam MessageType The type of message to parse.
 */
template <class MessageType>
class MessageParser {
  // Make sure we're actually parsing protobuf messages.
  static_assert(std::is_base_of<google::protobuf::Message, MessageType>::value,
                "Can only make parsers for protobuf messages.");

 public:
  /**
   * @brief Adds new serialized data to the parser.
   * @param data The data to add.
   * @note If a complete message has already been parsed, this method will
   *    do nothing until `GetMessage()` has been called.
   */
  void AddNewData(const std::vector<uint8_t>& data) {
    if (HasCompleteMessage()) {
      // Nothing to do.
      return;
    }

    size_t copy_offset = 0;
    if (partial_message_.empty()) {
      // This is the first chunk of data. Parse the message length.
      uint32_t message_size_network;
      std::copy(data.data(), data.data() + sizeof(message_size_network),
                &message_size_network);
      expected_length_ = ntohl(message_size_network);

      // Actual message data starts here.
      copy_offset = sizeof(message_size_network);
    }

    // Copy the actual message data.
    std::copy(data.begin() + copy_offset, data.end(), partial_message_.begin());
  }

  /**
   * @return True if a complete message has been parsed.
   */
  [[nodiscard]] bool HasCompleteMessage() const {
    return partial_message_.size() == expected_length_;
  }

  /**
   * @brief Gets the next message that has been parsed.
   * @param[out] message The message that it got.
   * @return True if it parsed the message, false if there was no complete
   *    message, or it couldn't be parsed.
   * @note Reading a complete message will clear internal data associated with
   *    that message.
   */
  bool GetMessage(MessageType* message) {
    if (!HasCompleteMessage()) {
      // No message to get.
      return false;
    }

    const bool kParseResult =
        message->ParseFromArray(partial_message_.data(), expected_length_);

    // Reset the internal state to prepare for new messages.
    partial_message_.clear();
    expected_length_ = std::numeric_limits<uint32_t>::max();

    return kParseResult;
  };

 private:
  /// The current partial message data.
  std::vector<uint8_t> partial_message_{};
  /// The length of the message we're currently reading.
  uint32_t expected_length_ = std::numeric_limits<uint32_t>::max();
};

}  // namespace wire_protocol

#endif  // PROJECT1_WIRE_PROTOCOL_H
