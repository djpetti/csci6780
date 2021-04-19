/**
 * @file IsConnected utilities for dealing with data sent over the network.
 */

#ifndef PROJECT1_WIRE_PROTOCOL_H
#define PROJECT1_WIRE_PROTOCOL_H

#include <arpa/inet.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

#include "google/protobuf/message.h"

namespace wire_protocol {

/**
 * @brief Serializes a message to the wire format. Once this is done, it can
 *  be safely sent over a socket.
 * @param message The message to serialize.
 * @param[out] serialized Will be set to the serialized output data. The size
 *  will be updated accordingly.
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
   */
  void AddNewData(const std::vector<uint8_t>& data) {
    uint32_t data_offset = ParseLength(data);
    data_offset += ParseMessage(data, data_offset);
    SaveOverflow(data, data_offset);
  }

  /**
   * @return True if a complete message has been parsed.
   */
  [[nodiscard]] bool HasCompleteMessage() const {
    return state_ == ParserState::PARSING_DONE;
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

    // Assume any extra data is the start of a new message.
    const std::vector<uint8_t> kNewMessageData(overflow_message_data_.begin(),
                                               overflow_message_data_.end());
    // Reset the internal state to prepare for new messages.
    ResetParser();
    // Parse the start of the next message.
    AddNewData(kNewMessageData);

    return kParseResult;
  };

  /**
   * @brief Resets the parser state.
   */
  void ResetParser() {
    state_ = ParserState::PARSING_LENGTH;
    got_length_bytes_ = 0;
    partial_message_.clear();
    expected_length_ = std::numeric_limits<uint32_t>::max();
    overflow_message_data_.clear();
  }

  /**
   * @return True if we have overflow data.
   */
  bool HasOverflow() {
    return !overflow_message_data_.empty();
  }

  /**
   * @return All data currently in the overflow buffer for this parser. This
   *    is data that belongs to the next message, and can be used to
   *    initialize a new parser.
   */
  [[nodiscard]] const std::vector<uint8_t>& GetOverflow() const {
    return overflow_message_data_;
  }

  /**
   * @return True if we have some partial message data.
   */
  bool HasPartialMessage() {
    return !partial_message_.empty() || got_length_bytes_ != 0;
  }

 private:
  /// Type we use to store the length in serialized messages.
  using MessageLengthType = uint32_t;
  /// Size of the length prefix to each message.
  static constexpr uint8_t kNumLengthBytes = sizeof(MessageLengthType);

  /// Keeps track of which state the parser is in.
  enum class ParserState {
    /// Parsing the length.
    PARSING_LENGTH,
    /// Parsing the message.
    PARSING_MESSAGE,
    /// Parsed a complete message.
    PARSING_DONE,
  };

  /**
   * @brief Handles parsing the length from the input data.
   * @param data The input data.
   * @return The number of bytes from the input that it consumed.
   */
  uint32_t ParseLength(const std::vector<uint8_t>& data) {
    if (state_ != ParserState::PARSING_LENGTH) {
      // We are not in the correct state for this.
      return 0;
    }

    // Copy any of the remaining length bytes.
    const uint32_t kLengthBytesToCopy =
        std::min(static_cast<uint32_t>(data.size()),
                 kNumLengthBytes - got_length_bytes_);
    std::copy(data.begin(), data.begin() + kLengthBytesToCopy,
              partial_length_.begin() + got_length_bytes_);
    got_length_bytes_ += kLengthBytesToCopy;

    if (got_length_bytes_ == kNumLengthBytes) {
      // We've parsed the entire length.
      state_ = ParserState::PARSING_MESSAGE;

      // Actually parse and save the length.
      MessageLengthType message_size_network;
      std::copy(partial_length_.begin(), partial_length_.end(),
                reinterpret_cast<uint8_t*>(&message_size_network));
      expected_length_ = ntohl(message_size_network);
    }

    return kLengthBytesToCopy;
  }

  /**
   * @brief Handles parsing the message from the input data.
   * @param data The input data.
   * @param start_offset The offset from the start of the input data to parse
   *    from.
   * @return The number of bytes from the input that it consumed.
   */
  uint32_t ParseMessage(const std::vector<uint8_t>& data,
                        uint32_t start_offset) {
    if (state_ != ParserState::PARSING_MESSAGE) {
      // We are not in the correct state for this.
      return 0;
    }

    // Determine number of bytes to copy.
    const uint32_t kRemainingData = data.size() - start_offset;
    const uint32_t kCurrentLength = partial_message_.size();
    const uint32_t kNumBytesToCopy =
        std::min(kRemainingData, expected_length_ - kCurrentLength);

    // Make sure we have enough space.
    partial_message_.resize(kCurrentLength + kNumBytesToCopy);
    // Copy the actual message data.
    std::copy(data.begin() + start_offset,
              data.begin() + start_offset + kNumBytesToCopy,
              partial_message_.begin() + kCurrentLength);

    // Check if this message is now complete.
    if (partial_message_.size() == expected_length_) {
      state_ = ParserState::PARSING_DONE;
    }

    return kNumBytesToCopy;
  }

  /**
   * @brief Handles parsing any overflow data that belongs to the next message.
   * @param data The input data.
   * @param start_offset The offset from the start of the input data to parse
   *    from.
   */
  void SaveOverflow(const std::vector<uint8_t>& data, uint32_t start_offset) {
    if (state_ != ParserState::PARSING_DONE) {
      // We are not in the correct state for this.
      return;
    }

    // Save any additional data for the next message.
    const uint32_t kRemainingData = data.size() - start_offset;
    const uint32_t kCurrentOverflowSize = overflow_message_data_.size();

    overflow_message_data_.resize(kCurrentOverflowSize + kRemainingData);
    std::copy(data.begin() + start_offset, data.end(),
              overflow_message_data_.begin() + kCurrentOverflowSize);
  }

  /// Tracks the current parser state.
  ParserState state_ = ParserState::PARSING_LENGTH;

  /// The current partial length data.
  std::array<uint8_t, kNumLengthBytes> partial_length_{};
  /// How many of the length bytes we've read so far.
  uint32_t got_length_bytes_ = 0;

  /// The current partial message data.
  std::vector<uint8_t> partial_message_{};
  /// The length of the message we're currently reading.
  uint32_t expected_length_ = std::numeric_limits<uint32_t>::max();

  /// Overflow data that belongs to the message after the current one.
  std::vector<uint8_t> overflow_message_data_{};
};

}  // namespace wire_protocol

#endif  // PROJECT1_WIRE_PROTOCOL_H
