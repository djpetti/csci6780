#include "chunked_file_sender.h"

#include <sys/socket.h>

#include <algorithm>
#include <loguru.hpp>

#include "../wire_protocol/wire_protocol.h"
#include "ftp_messages.pb.h"

using ftp_messages::FileContents;
using wire_protocol::Serialize;

namespace chunked_files {
namespace {

/// Number of bytes of the file to send in each chunk.
constexpr size_t kChunkSize = 1000;

}  // namespace

ChunkedFileSender::ChunkedFileSender(int socket) : socket_(socket) {}

void ChunkedFileSender::SetFileContents(const std::vector<uint8_t>& contents) {
  file_contents_ = contents;

  // Reset the state.
  total_bytes_sent_ = 0;
}

int ChunkedFileSender::SendNextChunk() {
  // Pack the next chunk into the outgoing buffer.
  FileContents file_message;
  const auto kActualSize =
      std::min(kChunkSize, file_contents_.size() - total_bytes_sent_);
  file_message.mutable_contents()->assign(
      file_contents_.begin() + total_bytes_sent_,
      file_contents_.begin() + total_bytes_sent_ + kActualSize);
  // Set if we're sending the last chunk.
  file_message.set_is_last(total_bytes_sent_ + kChunkSize >=
                           file_contents_.size());

  if (!Serialize(file_message, &outgoing_message_buffer_)) {
    LOG_S(ERROR) << "Failed to serialized message.";
    return -1;
  }

  // Send the message.
  size_t message_bytes_sent = 0;
  while (message_bytes_sent < outgoing_message_buffer_.size()) {
    const int kSendResult =
        send(socket_, outgoing_message_buffer_.data() + message_bytes_sent,
             outgoing_message_buffer_.size() - message_bytes_sent, 0);

    if (kSendResult < 0) {
      LOG_S(ERROR) << "Failed to send on socket.";
      return kSendResult;
    } else if (kSendResult == 0) {
      LOG_F(INFO, "Client with FD %i has disconnected.", socket_);
      return kSendResult;
    }

    message_bytes_sent += kSendResult;
  }

  // Successfully sent all the data.
  total_bytes_sent_ += kActualSize;
  return message_bytes_sent;
}

bool ChunkedFileSender::SentCompleteFile() const {
  return total_bytes_sent_ == file_contents_.size();
}

}  // namespace chunked_files
