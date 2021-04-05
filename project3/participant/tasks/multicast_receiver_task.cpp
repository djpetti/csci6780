#include "multicast_receiver_task.h"

#include <loguru.hpp>

namespace participant_tasks {
MulticastReceiver::MulticastReceiver(
    const std::shared_ptr<ConsoleTask>& console_task, std::string log_location,
    int port)
    : port_(port), log_location_(log_location), console_task_(console_task) {}

thread_pool::Task::Status MulticastReceiver::SetUp() {
  struct sockaddr_in address = participant_util::MakeAddress(port_);
  messenger_fd_ = participant_util::SetUpListenerSocket(address);
  loguru::add_file(log_location_.c_str(), loguru::FileMode::Truncate,
                   loguru::Verbosity_0);
  return thread_pool::Task::Status::RUNNING;
}

thread_pool::Task::Status MulticastReceiver::RunAtomic() {
  parser_.ResetParser();
  while (!parser_.HasCompleteMessage()) {
    incoming_msg_buf_.resize(kBufferSize);
    const auto bytes_read = participant_util::ReceiveForever(
        messenger_fd_, incoming_msg_buf_.data(), kBufferSize, 0);
    if (bytes_read <= 0) {
      break;
    }
    incoming_msg_buf_.resize(bytes_read);
    parser_.AddNewData(incoming_msg_buf_);
  }
  // empty buffer
  incoming_msg_buf_.clear();
  if (parser_.HasCompleteMessage()) {
    pub_sub_messages::ForwardMulticast msg;
    parser_.GetMessage(&msg);
    std::string to_out =
        "[" + std::to_string(msg.origin_id()) + "] " + msg.message();
    LOG_S(0) << to_out;
    console_task_->SendConsole(to_out);
  }
  return thread_pool::Task::Status::RUNNING;
}

void MulticastReceiver::CleanUp() { close(messenger_fd_); }

}  // namespace participant_tasks
