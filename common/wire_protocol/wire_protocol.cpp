#include "wire_protocol.h"

namespace wire_protocol {

bool Serialize(const google::protobuf::Message& message,
               std::vector<uint8_t>* serialized) {
  // Compute and pack the size.
  const uint32_t kMessageSize = message.ByteSizeLong();
  const uint32_t kMessageSizeNetwork = htonl(kMessageSize);
  serialized->resize(sizeof(kMessageSize) + kMessageSize);
  std::copy(reinterpret_cast<const uint8_t*>(&kMessageSizeNetwork),
            reinterpret_cast<const uint8_t*>(&kMessageSizeNetwork + 1),
            serialized->data());

  // Serialize the actual message.
  return message.SerializeToArray(
      serialized->data() + sizeof(kMessageSizeNetwork), kMessageSize);
}

}  // namespace wire_protocol
