#include "client.h"

#include <netinet/in.h>
#include <sys/socket.h>

#include <iostream>

namespace {

/**
 * @brief helper function for making a socket address structure
 *
 * @param port FTP server Port #
 */
struct sockaddr_in MakeAddress(uint16_t port) {
  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  return address;
}

/**
 * @brief connects to socket
 * @param address socket address structure
 * @param hostname FTP server IP address
 * @return socket fd on success, -1 on failure
 *
 */
int SetUpSocket(const struct sockaddr_in &address,
                const std::string &hostname) {
  int sock = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation error");
    return -1;
  }

  if (inet_pton(AF_INET, hostname.c_str(),
                (struct sockaddr *)&address.sin_addr) <= 0) {
    perror("Invalid Address or Address not supported");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Connection Failed");
    return -1;
  }

  return sock;
}
}  // namespace

namespace client {

using client::input_parser::InputParser;

bool Client::Connect(const std::string &hostname, uint16_t port) {
  client_fd_ = SetUpSocket(MakeAddress(port), hostname);
  connected_ = true;
  return connected_;
}

bool Client::SendReq() {
  if (!connected_) {
    perror("Cannot Send Request... no longer connected to server");
    return false;
  }
  if (send(client_fd_, outgoing_msg_buf_.data(), outgoing_msg_buf_.size(), 0) <
      0) {
    perror("Failed to send request");
    return false;
  }
  // empty buffer
  outgoing_msg_buf_.clear();

  return true;
}

bool Client::WaitForResponse() {
  while (!parser_.HasCompleteMessage()) {
    incoming_msg_buf_.resize(kBufferSize);

    const auto bytes_read =
        recv(client_fd_, incoming_msg_buf_.data(), kBufferSize, 0);
    if (bytes_read < 0) {
      connected_ = false;
    } else if (bytes_read == 0) {
      connected_ = false;
    }

    incoming_msg_buf_.resize(bytes_read);

    parser_.AddNewData(incoming_msg_buf_);
  }

  // empty buffer
  incoming_msg_buf_.clear();

  return connected_;
}

void Client::HandleResponse() {
  output_ = "";

  ftp_messages::Response msg;

  parser_.GetMessage(&msg);

  // determine type of response and respond accordingly
  if (msg.has_get()) {
    auto g_response = msg.get();

    std::ofstream new_file;
    new_file.open(ip_->GetFilename());
    new_file << g_response.file_contents();
  } else if (msg.has_list()) {
    auto l_response = msg.list();

    // append filenames to output string separated by whitespace
    for (int i = 0; i < l_response.filenames_size(); i++) {
      output_.append(l_response.filenames(i));
      if (i != l_response.filenames_size() - 1) {
        output_.append(" ");
      }
    }
  } else if (msg.has_pwd()) {
    auto pwd_response = msg.pwd();
    output_ = pwd_response.dir_name();
  }

  // make sure outputting is necessary
  if (output_.compare("")) {
    Output();
  }

  delete ip_;
}

void Client::Output() {
  std::istringstream iss(output_);
  int i = 0;

  // might be unnecessary
  std::cout << "\n";

  do {
    std::string word;
    iss >> word;
    std::cout << word + "   ";
    if (i == 3) {
      std::cout << "\n";
      i = 0;
    }
    i++;
  } while (iss);
}
bool Client::FtpShell() {
  ftp_messages::Request r;

  while (connected_) {
    // reset input string
    std::string input;

    // will have to pay attention to formatting here when implementing Output()
    std::cout << "\nmyftp> ";

    // get input
    std::getline(std::cin, input);

    // determine command
    ip_ = new InputParser(input);

    // create & serialize request message for determined command
    r = ip_->CreateReq();
    wire_protocol::Serialize(r, &outgoing_msg_buf_);

    if (r.has_quit()) {
      // close socket connection
      close(client_fd_);

      delete ip_;

      // exit loop, no need to send request
      connected_ = false;
      continue;
    }
    SendReq();
    WaitForResponse();
    HandleResponse();
  }
  return true;
}
}  // namespace client