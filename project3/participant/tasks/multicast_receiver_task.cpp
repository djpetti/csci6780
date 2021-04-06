#include "multicast_receiver_task.h"

#include <loguru.hpp>

namespace participant_tasks {
MulticastReceiver::MulticastReceiver(
    const std::shared_ptr<ConsoleTask>& console_task,
    std::filesystem::path log_location, int port)
    : port_(port), log_location_(log_location), console_task_(console_task) {}

thread_pool::Task::Status MulticastReceiver::SetUp() {
  struct sockaddr_in address = participant_util::MakeAddress(port_);
  messenger_fd_ = participant_util::SetUpListenerSocket(address);
  return thread_pool::Task::Status::RUNNING;
}

thread_pool::Task::Status MulticastReceiver::RunAtomic() {
  // If parser_ has a completed message ready
  if (parser_.HasCompleteMessage()) {
    pub_sub_messages::ForwardMulticast msg;
    parser_.GetMessage(&msg);
    std::string to_out =
        "[" + std::to_string(msg.origin_id()) + "] " + msg.message();
    WriteLineToLog(to_out);
    console_task_->SendConsole(to_out);
    incoming_msg_buf_.clear();
  // Else, attempt to parse anything new
  } else {
    incoming_msg_buf_.resize(kBufferSize);
    const auto bytes_read = participant_util::ReceiveForever(
        messenger_fd_, incoming_msg_buf_.data(), kBufferSize, 0);
    if (bytes_read < 0) {
      LOG_S(0) << "Reading from socket failed.";
      return thread_pool::Task::Status::FAILED;
    } else if (bytes_read == 0) {
      LOG_S(0) << "Socket closed.";
      return thread_pool::Task::Status::DONE;
    }
    incoming_msg_buf_.resize(bytes_read);
    parser_.AddNewData(incoming_msg_buf_);
  }
  return thread_pool::Task::Status::RUNNING;
}

void MulticastReceiver::CleanUp() { close(messenger_fd_); }

void MulticastReceiver::WriteLineToLog(const std::string& message) {
  std::ofstream log_file;
  log_file.open(log_location_, std::ios_base::app);
  log_file << message << std::endl;
}

}  // namespace participant_tasks
