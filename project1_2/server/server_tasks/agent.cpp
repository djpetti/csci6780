#include "agent.h"

#include <sys/socket.h>
#include <unistd.h>

#include <loguru.hpp>
#include <string>
#include <thread>
#include <utility>

#include "chunked_files/chunked_file_receiver.h"
#include "chunked_files/chunked_file_sender.h"

namespace server {

using file_handler::ThreadSafeFileHandler;
using ftp_messages::Request;
using ftp_messages::Response;

namespace {

/// Generic empty response.
const Response kEmptyResponse{};

}  // namespace

Agent::Agent(int client_fd, std::unique_ptr<ThreadSafeFileHandler> file_handler,
             std::shared_ptr<server_tasks::CommandIDs> active_commands)
    : client_fd_(client_fd),
      active_commands_(std::move(active_commands)),
      file_handler_(std::move(file_handler)) {}

Agent::Agent(int client_fd,
             std::shared_ptr<server_tasks::CommandIDs> active_commands)
    : client_fd_(client_fd), active_commands_(std::move(active_commands)) {}

Agent::~Agent() {
  // Close the socket.
  close(client_fd_);
}

bool Agent::Handle() {
  ClientState client_state = ClientState::ACTIVE;

  while (client_state == ClientState::ACTIVE) {
    // Get the next message from the socket.
    Request message;
    client_state = ReadNextMessage(&message);

    if (client_state == ClientState::ACTIVE) {
      // Handle the message.
      client_state = DispatchMessage(message);
    }
  }

  return client_state != ClientState::ERROR;
}

Agent::ClientState Agent::ReadFileContents(std::vector<uint8_t> *file_contents,
                                           uint16_t command_id) {
  chunked_files::ChunkedFileReceiver receiver(client_fd_);

  while (!receiver.HasCompleteFile()) {
    bool terminated = !active_commands_->Contains(command_id);
    incoming_message_buffer_.resize(kClientBufferSize);

    // Read 1000 bytes from the socket
    const auto bytes_read = receiver.ReceiveNextChunk();

    if (bytes_read < 0) {
      // Failed to read anything.
      return ClientState::ERROR;
    } else if (bytes_read == 0) {
      // Client has disconnected nicely.
      return ClientState::DISCONNECTED;
    }

    // only continue if this put request is an active command
    if (terminated) {
      // Make sure we leave the socket in a valid state.
      if (!receiver.CleanUp()) {
        return ClientState::ERROR;
      }
      LOG_F(INFO, "Command #%i successfully terminated.", command_id);
      return ClientState::ACTIVE;
    }
  }

  // Get the parsed message.
  receiver.GetFileContents(file_contents);
  return ClientState::ACTIVE;
}

Agent::ClientState Agent::ReadNextMessage(Request *message) {
  while (!parser_.HasCompleteMessage()) {
    incoming_message_buffer_.resize(kClientBufferSize);

    // Read some more data from the socket.
    const auto bytes_read =
        recv(client_fd_, incoming_message_buffer_.data(), kClientBufferSize, 0);

    if (bytes_read < 0) {
      // Failed to read anything.
      LOG_F(ERROR, "Failed to read from client (%i) socket.", client_fd_);
      parser_.ResetParser();
      return ClientState::ERROR;
    } else if (bytes_read == 0) {
      // Client has disconnected nicely.
      LOG_F(INFO, "Client with FD %i has disconnected.", client_fd_);
      return ClientState::DISCONNECTED;
    }

    // The parser assumes that the entire vector contains valid data, so limit
    // the size.
    incoming_message_buffer_.resize(bytes_read);

    // Add the data to the parser.
    parser_.AddNewData(incoming_message_buffer_);
  }

  // Get the parsed message.
  if (!parser_.GetMessage(message)) {
    LOG_F(ERROR, "Failed to get the parsed message from client (%i).",
          client_fd_);
    return ClientState::ERROR;
  }
  return ClientState::ACTIVE;
}

Agent::ClientState Agent::DispatchMessage(const Request &message) {
  if (message.has_get()) {
    return HandleRequest(message.get());
  } else if (message.has_put()) {
    return HandleRequest(message.put());
  } else if (message.has_delete_()) {
    return HandleRequest(message.delete_());
  } else if (message.has_list()) {
    return HandleRequest(message.list());
  } else if (message.has_change_dir()) {
    return HandleRequest(message.change_dir());
  } else if (message.has_make_dir()) {
    return HandleRequest(message.make_dir());
  } else if (message.has_pwd()) {
    return HandleRequest(message.pwd());
  } else if (message.has_quit()) {
    return HandleRequest(message.quit());
  } else if (message.has_terminate()) {
    return HandleRequest(message.terminate());
  } else if (message.has_file_contents()) {
    // Spurious file_contents message. Ignore.
    LOG_S(1) << "Ignoring spurious file_contents message.";
    return ClientState::ACTIVE;
  }

  LOG_F(ERROR, "No valid message from client (%i) was recieved.", client_fd_);
  return ClientState::ERROR;
}

bool Agent::SendResponse(const Response &response) {
  // Serialize the message.
  if (!wire_protocol::Serialize(response, &outgoing_message_buffer_)) {
    LOG_F(ERROR, "Failed to serialize message.");
    return false;
  }

  // Send the message.
  if (send(client_fd_, outgoing_message_buffer_.data(),
           outgoing_message_buffer_.size(), 0) < 0) {
    LOG_F(ERROR, "Failed to send message.");
    return false;
  }
  return true;
}

bool Agent::SendFileContents(const std::vector<uint8_t> &file_contents,
                             uint16_t command_id) {
  chunked_files::ChunkedFileSender sender(client_fd_);
  sender.SetFileContents(file_contents);

  // monitor for termination request for get and put commands
  uint32_t total_bytes_sent = 0;
  bool terminated = false;

  // continue until we've sent the entire message
  while (!sender.SentCompleteFile()) {
    if (!terminated) {
      int bytes_sent = sender.SendNextChunk();
      if (bytes_sent != -1) {
        total_bytes_sent += bytes_sent;
      } else {
        return false;
      }
    } else {
      LOG_F(INFO, "Command #%i for client #%i successfully terminated.",
            command_id, client_fd_);
      return true;
    }
    terminated = !active_commands_->Contains(command_id);
  }
  return true;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::GetRequest &request) {
  LOG_F(INFO, "Performing a GET operation for client (%i).", client_fd_);

  Response r;

  // generate a random # for the command id
  uint32_t id = active_commands_->GenerateID();

  // register this command as an active command
  active_commands_->Insert(id);

  // make response
  r.mutable_get()->set_command_id(id);

  // Send client the command id for possible termination
  SendResponse(r);

  // Get the file data.
  const auto file_data = file_handler_->Get(request.filename());

  // Send the FileContents.
  const auto kSendResult = SendFileContents(file_data, id);

  active_commands_->Delete(id);
  return kSendResult ? ClientState::ACTIVE : ClientState::ERROR;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::PutRequest &request) {
  LOG_F(INFO, "Performing a PUT operation for client (%i).", client_fd_);

  Response r;
  uint32_t id = active_commands_->GenerateID();
  // register this command as an active command
  active_commands_->Insert(id);
  r.mutable_put()->set_command_id(id);

  // send client command id for possible termination
  SendResponse(r);

  // file contents to Put
  ftp_messages::FileContents fc;

  // Retrieve the file contents
  std::vector<uint8_t> file_contents;
  ReadFileContents(&file_contents, id);

  // Write the file data.
  if (!file_handler_->Put(request.filename(), file_contents)) {
    std::string fn = request.filename();
    LOG_F(ERROR, "Failed to write the file (%s) for client (%i).", fn.c_str(),
          client_fd_);
    return ClientState::ERROR;
  }
  active_commands_->Delete(id);
  return ClientState::ACTIVE;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::DeleteRequest &request) {
  LOG_F(INFO, "Performing a DELETE operation for client (%i).", client_fd_);

  // Delete the file.
  if (!file_handler_->Delete(request.filename())) {
    std::string fn = request.filename();
    LOG_F(ERROR, "Failed to delete the file (%s) for client (%i).", fn.c_str(),
          client_fd_);
    return ClientState::ERROR;
  }

  return SendResponse(kEmptyResponse) ? ClientState::ACTIVE
                                      : ClientState::ERROR;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::ListRequest &request) {
  LOG_F(INFO, "Performing a LIST operation for client (%i).", client_fd_);

  // List the directory contents.
  const auto &kDirectoryContents = file_handler_->List();

  // Send the response.
  Response response;
  response.mutable_list()->clear_filenames();
  for (const auto &kFilename : kDirectoryContents) {
    response.mutable_list()->add_filenames(kFilename);
  }
  return SendResponse(response) ? ClientState::ACTIVE : ClientState::ERROR;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::ChangeDirRequest &request) {
  LOG_F(INFO, "Performing a PUT operation for client (%i).", client_fd_);

  if (request.go_up()) {
    // Go up a directory.
    if (!file_handler_->UpDir()) {
      LOG_F(ERROR, "Failed to go up a directory for client (%i).", client_fd_);
      return ClientState::ERROR;
    }
  } else {
    // Go down a directory.
    if (!file_handler_->ChangeDir(request.dir_name())) {
      LOG_F(ERROR, "Failed to go down a directory for client (%i).",
            client_fd_);
      return ClientState::ERROR;
    }
  }

  return SendResponse(kEmptyResponse) ? ClientState::ACTIVE
                                      : ClientState::ERROR;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::MakeDirRequest &request) {
  LOG_F(INFO, "Performing a MKDIR operation for client (%i).", client_fd_);

  // Create the directory.
  if (!file_handler_->MakeDir(request.dir_name())) {
    LOG_F(ERROR, "Failed to create directory (%s) for client (%i).",
          request.dir_name().c_str(), client_fd_);
    return ClientState::ERROR;
  }

  return SendResponse(kEmptyResponse) ? ClientState::ACTIVE
                                      : ClientState::ERROR;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::PwdRequest &request) {
  LOG_F(INFO, "Performing a PWD operation for client (%i).", client_fd_);

  // Get the directory.
  const auto &kCurrentDir = file_handler_->GetCurrentDir();

  // Send the response.
  Response response;
  response.mutable_pwd()->set_dir_name(kCurrentDir);
  return SendResponse(response) ? ClientState::ACTIVE : ClientState::ERROR;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::TerminateRequest &request) {
  // Retrieve the command id.
  const uint32_t kCommandId = request.command_id();
  // remove command from active command list.
  active_commands_->Delete(kCommandId);

  return ClientState::ACTIVE;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::QuitRequest &request) {
  LOG_F(INFO, "Quit request for client (%i).", client_fd_);
  // Indicate that we are finished with this client.
  return ClientState::DISCONNECTED;
}

}  // namespace server
