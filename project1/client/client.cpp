#include "client.h"
#include "client_tasks/terminate_task.h"
#include "client_tasks/upload_task.h"
#include "client_tasks/download_task.h"
#include "../thread_pool/thread_pool.h"

namespace client {

using client::input_parser::InputParser;

bool Client::Connect(const std::string &hostname, uint16_t nport, uint16_t tport) {
  client_fd_ = client_util::SetUpSocket(client_util::MakeAddress(nport), hostname);
  hostname_ = hostname;
  nport_ = nport;
  tport_ = tport;
  connected_ = client_fd_ != -1;
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

bool Client::WaitForMessage() {
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
  if (msg.has_list()) {
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
  } else if (msg.has_put()) {
    auto put_response = msg.put();
    output_ = put_response.command_id();
  } else if (msg.has_get()) {
    auto get_response = msg.get();
    output_ = get_response.command_id();
  }

  // make sure outputting is necessary
  if (!output_.empty()) {
    Output();
  }
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
  thread_pool::ThreadPool pool;

  while (connected_) {
    // will have to pay attention to formatting here when implementing Output()
    std::cout << "\nmyftp> ";

    // reset input string
    std::string input;

    // get input
    std::getline(std::cin, input);

    // determine command
    ip_ = new InputParser(input);

    // create & serialize request message for determined command
    r = ip_->CreateReq();
    if (!ip_->IsValid()) {
      std::cout << "Invalid command, try again." << "\n";
      continue;
    }
    if (r.has_terminate()) {
      auto terminate_task = std::make_shared<client_tasks::TerminateTask>(hostname_, tport_, r.terminate());
      pool.AddTask(terminate_task);
      delete ip_;
      continue;
    }
    if (r.has_quit()) {
      // close socket connection
      close(client_fd_);
      delete ip_;
      // exit loop, no need to send request
      connected_ = false;
      continue;
    }
    wire_protocol::Serialize(r, &outgoing_msg_buf_);
    SendReq();
    WaitForMessage();
    HandleResponse();
    if (r.has_put()) {
      // If put command, FileContents will have to be sent.
      auto contents = ip_->GetContentsMessage();
      wire_protocol::Serialize(contents, &outgoing_msg_buf_);
      if (!ip_->IsForking()) {
        SendReq();
      } else {
        auto put_task = std::make_shared<client_tasks::UploadTask>(client_fd_, outgoing_msg_buf_);
        pool.AddTask(put_task);
      }
    } else if (r.has_get()) {
      // If get command, FileContents will have to be received.
      if (!ip_->IsForking()) {
        WaitForMessage();
        client_util::SaveIncomingFile(parser_, ip_->GetFilename());
      } else {
        auto get_task = std::make_shared<client_tasks::DownloadTask>(ip_->GetFilename(), incoming_msg_buf_, kBufferSize, client_fd_, parser_);
        pool.AddTask(get_task);
      }
    }
    delete ip_;
  }
  return true;
}
}  // namespace client