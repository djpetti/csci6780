#include "agent.h"

#include <sys/socket.h>

#include <cstdio>
#include <iostream>

namespace server {

using file_handler::IFileHandler;
using ftp_messages::Request;
using ftp_messages::Response;

Agent::Agent(int client_fd, std::unique_ptr<IFileHandler> file_handler)
    : client_fd_(client_fd), file_handler_(std::move(file_handler)) {}

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

Agent::ClientState Agent::ReadNextMessage(Request* message) {
  while (!parser_.HasCompleteMessage()) {
    incoming_message_buffer_.resize(kClientBufferSize);

    // Read some more data from the socket.
    const auto bytes_read =
        recv(client_fd_, incoming_message_buffer_.data(), kClientBufferSize, 0);
    if (bytes_read < 0) {
      // Failed to read anything.
      perror("Failed to read from client socket");
      parser_.ResetParser();
      return ClientState::ERROR;
    } else if (bytes_read == 0) {
      // Client has disconnected nicely.
      std::cout << "Detected client disconnect." << std::endl;
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
    std::cerr << "Failed to get the parsed message." << std::endl;
    return ClientState::ERROR;
  }
  return ClientState::ACTIVE;
}

Agent::ClientState Agent::DispatchMessage(const Request& message) {
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
  }

  std::cerr << "No valid message was received." << std::endl;
  return ClientState::ERROR;
}

bool Agent::SendResponse(const Response& response) {
  // Serialize the message.
  if (!wire_protocol::Serialize(response, &outgoing_message_buffer_)) {
    std::cerr << "Failed to serialize message." << std::endl;
    return false;
  }

  // Send the message.
  if (send(client_fd_, outgoing_message_buffer_.data(),
           outgoing_message_buffer_.size(), 0) < 0) {
    perror("Failed to send the message");
    return false;
  }
  return true;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::GetRequest& request) {
  std::cout << "Performing a GET operation." << std::endl;

  // Get the file data.
  const auto file_data = file_handler_->Get(request.filename());

  // Send the response.
  Response response;
  response.mutable_get()->set_file_contents(file_data.data(), file_data.size());
  return SendResponse(response) ? ClientState::ACTIVE : ClientState::ERROR;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::PutRequest& request) {
  std::cout << "Performing a PUT operation." << std::endl;

  // Write the file data.
  std::vector<uint8_t> file_contents(request.file_contents().begin(),
                                     request.file_contents().end());
  if (!file_handler_->Put(request.filename(), file_contents)) {
    std::cerr << "Failed to write the file." << std::endl;
    return ClientState::ERROR;
  }
  return ClientState::ACTIVE;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::DeleteRequest& request) {
  std::cout << "Performing a DELETE request." << std::endl;

  // Delete the file.
  if (!file_handler_->Delete(request.filename())) {
    std::cerr << "Failed to delete the file." << std::endl;
    return ClientState::ERROR;
  }
  return ClientState::ACTIVE;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::ListRequest& request) {
  std::cout << "Performing a LIST request." << std::endl;

  // List the directory contents.
  const auto& kDirectoryContents = file_handler_->List();

  // Send the response.
  Response response;
  response.mutable_list()->clear_filenames();
  for (const auto& kFilename : kDirectoryContents) {
    response.mutable_list()->add_filenames(kFilename);
  }
  return SendResponse(response) ? ClientState::ACTIVE : ClientState::ERROR;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::ChangeDirRequest& request) {
  std::cout << "Performing a CD request." << std::endl;

  if (request.go_up()) {
    // Go up a directory.
    if (!file_handler_->UpDir()) {
      std::cerr << "Failed to go up a directory." << std::endl;
      return ClientState::ERROR;
    }
  } else {
    // Go down a directory.
    if (!file_handler_->ChangeDir(request.dir_name())) {
      std::cerr << "Failed to go down a directory." << std::endl;
      return ClientState::ERROR;
    }
  }

  return ClientState::ACTIVE;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::MakeDirRequest& request) {
  std::cout << "Performing a MKDIR request." << std::endl;

  // Create the directory.
  if (!file_handler_->MakeDir(request.dir_name())) {
    std::cerr << "Failed to create a directory." << std::endl;
    return ClientState::ERROR;
  }
  return ClientState::ACTIVE;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::PwdRequest& request) {
  std::cout << "Performing a PWD request." << std::endl;

  // Get the directory.
  const auto& kCurrentDir = file_handler_->GetCurrentDir();

  // Send the response.
  Response response;
  response.mutable_pwd()->set_dir_name(kCurrentDir);
  return SendResponse(response) ? ClientState::ACTIVE : ClientState::ERROR;
}

Agent::ClientState Agent::HandleRequest(
    const ftp_messages::QuitRequest& request) {
  std::cout << "Got a QUIT request. Exiting." << std::endl;
  // Indicate that we are finished with this client.
  return ClientState::DISCONNECTED;
}

}  // namespace server
