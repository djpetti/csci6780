#include "chunked_file_receiver.h"

#include <sys/socket.h>

#include <loguru.hpp>

using ftp_messages::FileContents;

namespace chunked_files {
namespace {

/// Size to receive message chunks in.
constexpr uint32_t kClientBufferSize = 4096;

}  // namespace

ChunkedFileReceiver::ChunkedFileReceiver(int socket) : socket_(socket) {}

int ChunkedFileReceiver::ReceiveNextChunk() {
  uint32_t total_bytes_read = 0;

  while (!parser_.HasCompleteMessage()) {
    incoming_message_buffer_.resize(kClientBufferSize);

    // Read 1000 bytes from the socket
    const auto bytes_read =
        recv(socket_, incoming_message_buffer_.data(), kClientBufferSize, 0);

    if (bytes_read < 0) {
      // Failed to read anything.
      LOG_F(ERROR, "Failed to read from client socket.");
      return bytes_read;
    } else if (bytes_read == 0) {
      // Client has disconnected nicely.
      LOG_F(INFO, "Client with FD %i has disconnected.", socket_);
      return bytes_read;
    }

    // The parser assumes that the entire vector contains valid data, so limit
    // the size.
    incoming_message_buffer_.resize(bytes_read);
    total_bytes_read += bytes_read;
    // Add the data to the parser.
    parser_.AddNewData(incoming_message_buffer_);
  }

  // Get the parsed message.
  FileContents file_message;
  if (!parser_.GetMessage(&file_message)) {
    LOG_F(ERROR, "Failed to get the parsed message from client (%i).", socket_);
    return -1;
  }
  // Add it to the internal buffer.
  file_contents_ += file_message.contents();

  if (file_message.is_last()) {
    // We found the end of the file.
    complete_file_ = true;
  }

  return file_message.contents().size();
}

bool ChunkedFileReceiver::HasCompleteFile() const { return complete_file_; }

void ChunkedFileReceiver::GetFileContents(std::vector<uint8_t> *contents) {
  contents->clear();
  contents->assign(file_contents_.begin(), file_contents_.end());

  Reset();
}

void ChunkedFileReceiver::Reset() {
  parser_.ResetParser();

  incoming_message_buffer_.clear();
  file_contents_.clear();

  complete_file_ = false;
}

bool ChunkedFileReceiver::CleanUp() {
  while (parser_.HasOverflow() || parser_.HasPartialMessage()) {
    if (ReceiveNextChunk() < 0) {
      // Failure to read.
      return false;
    }
  }

  Reset();
  return true;
}

}  // namespace chunked_files