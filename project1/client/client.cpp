#include "client.h"

#include <memory>
#include <sstream>
#include <chrono>

#include "../thread_pool/thread_pool.h"
#include "client_tasks/download_task.h"
#include "client_tasks/terminate_task.h"
#include "client_tasks/upload_task.h"

namespace client {

using client::input_parser::InputParser;

bool Client::Connect(const std::string &hostname, uint16_t nport,
                     uint16_t tport) {
  client_fd_ =
      client_util::SetUpSocket(client_util::MakeAddress(nport), hostname);
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
  parser_.ResetParser();
  while (!parser_.HasCompleteMessage()) {
    incoming_msg_buf_.resize(kBufferSize);

    // ONLY after successful termination of a forked GET request
    // subsequent 'LIST' and 'PWD' requests
    // fail at this line on the first iteration of this loop
    // recv will read an enormous amount of bytes and program freezes,  or it reads -1, then on the next subsequent command reads an enormous amount bytes
    // enormous amount of bytes: 140732703310832
    const auto bytes_read =
        recv(client_fd_, incoming_msg_buf_.data(), kBufferSize, 0);
    if (bytes_read < 0) {
      break;
    }
    else if (bytes_read == 0) {
        break;
    }

    incoming_msg_buf_.resize(bytes_read);

    parser_.AddNewData(incoming_msg_buf_);
  }

  // empty buffer
  incoming_msg_buf_.clear();

  return connected_;
}

void Client::HandleResponse() {
  ftp_messages::Response msg;

  parser_.GetMessage(&msg);

  // determine type of response and respond accordingly
  if (msg.has_list()) {
    auto l_response = msg.list();
    for (int i = 0, step = 1; i < l_response.filenames_size(); i++, step++) {
      std::cout << l_response.filenames(i);
      if (step % 3 == 0 && i != l_response.filenames_size() - 1) {
        std::cout << "\n";
      } else {
        std::cout << "   ";
      }
    }
  } else if (msg.has_pwd()) {
    auto pwd_response = msg.pwd();
    std::cout << pwd_response.dir_name();
  } else if (msg.has_put()) {
    auto put_response = msg.put();
    std::cout << "command_id: " << std::to_string(put_response.command_id());
  } else if (msg.has_get()) {
    auto get_response = msg.get();
    std::cout << "command_id: " << std::to_string(get_response.command_id());
  }
}

void Client::FtpShell() {
  ftp_messages::Request r;
  ftp_messages::Request r_old;
  std::shared_ptr<client_tasks::UploadTask> put_task;
  std::shared_ptr<client_tasks::DownloadTask> get_task;
  thread_pool::ThreadPool pool;
  while (connected_) {
    // display myftp prompt
    std::cout << "\nmyftp> ";

    // reset input string
    std::string input;

    // get input
    std::getline(std::cin, input);

    // determine command
    std::unique_ptr<input_parser::InputParser> ip =
        std::make_unique<input_parser::InputParser>(input);

    // create & serialize request message for determined command
    r = ip->CreateReq();



    if (!ip->IsValid()) {
      std::cout << "Invalid command, try again."
                << "\n";
      continue;
    }
    if (r.has_terminate()) {
      auto terminate_task = std::make_shared<client_tasks::TerminateTask>(
          hostname_, tport_, r.terminate());
      pool.AddTask(terminate_task);

      if (r_old.has_get()) {
          pool.CancelTask(get_task);
      } else if (r_old.has_put()){
          pool.CancelTask(put_task);
      }
      continue;
    }
    if (r.has_quit()) {
      // close socket connection
      close(client_fd_);
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
      auto contents = ip->GetContentsMessage();
      wire_protocol::Serialize(contents, &outgoing_msg_buf_);
      if (!ip->IsForking()) {
        SendReq();
      } else {

          put_task = std::make_shared<client_tasks::UploadTask>(
                  client_fd_, outgoing_msg_buf_);
        pool.AddTask(put_task);
      }
    } else if (r.has_get()) {
      // If get command, FileContents will have to be received.
      if (!ip->IsForking()) {
        WaitForMessage();
        ftp_messages::FileContents contents;
        parser_.GetMessage(&contents);
        client_util::SaveIncomingFile(contents.contents(), ip->GetFilename());
      } else {
        get_task = std::make_shared<client_tasks::DownloadTask>(
            ip->GetFilename(), kBufferSize, client_fd_);
        pool.AddTask(get_task);

      }
      r_old = r;
    }
  }
}
}  // namespace client