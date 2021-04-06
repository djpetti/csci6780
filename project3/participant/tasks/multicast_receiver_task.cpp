#include "multicast_receiver_task.h"

#include <sys/socket.h>

#include <cerrno>
#include <cstring>
#include <loguru.hpp>
#include <utility>

namespace participant_tasks {
namespace {

/// Timeout in seconds for socket operations.
constexpr uint32_t kSocketTimeout = 1;

}  // namespace

MulticastReceiver::MulticastReceiver(std::shared_ptr<ConsoleTask> console_task,
                                     std::string log_location, int port)
    : port_(port),
      log_location_(std::move(log_location)),
      console_task_(std::move(console_task)) {}

thread_pool::Task::Status MulticastReceiver::SetUp() {
  struct sockaddr_in address = participant_util::MakeAddress(port_);
  server_fd_ = participant_util::SetUpListenerSocket(address);
  loguru::add_file(log_location_.c_str(), loguru::FileMode::Truncate,
                   loguru::Verbosity_0);

  // Wait for and accept the connection from the coordinator.
  messenger_fd_ = accept(server_fd_, nullptr, nullptr);
  if (messenger_fd_ < 0) {
    LOG_S(ERROR) << "Failed to accept coordinator connection: "
                 << std::strerror(errno);
    return thread_pool::Task::Status::FAILED;
  }
  LOG_S(INFO) << "Accepted connection from coordinator.";

  // Set a timeout.
  struct timeval timeout {};
  timeout.tv_sec = kSocketTimeout;
  timeout.tv_usec = 0;
  setsockopt(messenger_fd_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout,
             sizeof(timeout));

  return thread_pool::Task::Status::RUNNING;
}

thread_pool::Task::Status MulticastReceiver::RunAtomic() {
  // If parser_ has a completed message ready
  if (parser_.HasCompleteMessage()) {
    pub_sub_messages::ForwardMulticast msg;
    parser_.GetMessage(&msg);
    std::string to_out =
        "[" + std::to_string(msg.origin_id()) + "] " + msg.message();
    LOG_S(0) << to_out;
    console_task_->SendConsole(to_out);
    incoming_msg_buf_.clear();
    // Else, attempt to parse anything new
  } else {
    incoming_msg_buf_.resize(kBufferSize);

    const auto bytes_read =
        recv(messenger_fd_, incoming_msg_buf_.data(), kBufferSize, 0);
    if (bytes_read < 0) {
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
        // This is merely a timeout, and we should just spin again.
        return thread_pool::Task::Status::RUNNING;
      }

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

void MulticastReceiver::CleanUp() {
  close(messenger_fd_);
  close(server_fd_);
}

}  // namespace participant_tasks
