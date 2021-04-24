/**
 * @file Some common types.
 */

#ifndef CSCI6780_TYPES_H
#define CSCI6780_TYPES_H

#include <cstdint>
#include <functional>
#include <string>

namespace message_passing {

/// Message ID type.
using MessageId = uint64_t;

/**
 * @brief Represents an endpoint to send messages to.
 */
struct Endpoint {
  /// The destination host.
  std::string hostname;
  /// The destination port.
  uint16_t port;
};

/// Custom hash specialization for endpoints.
struct EndpointHash {
  std::size_t operator()(
      const message_passing::Endpoint& endpoint) const noexcept {
    return std::hash<std::string>{}(endpoint.hostname) ^
           std::hash<uint16_t>{}(endpoint.port);
  }
};

/// Equality operator for endpoints.
inline bool operator==(const Endpoint& e1, const Endpoint& e2) {
  return e1.hostname == e2.hostname && e1.port == e2.port;
}

/// Inequality operator for endpoints.
inline bool operator!=(const Endpoint& e1, const Endpoint& e2) {
  return !(e1 == e2);
}

}  // namespace message_passing

#endif  // CSCI6780_TYPES_H
