/**
 * @file Participant messaging task
 */
#ifndef PROJECT3_MULTICAST_RECEIVER_H
#define PROJECT3_MULTICAST_RECEIVER_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pub_sub_messages.pb.h>
#include <sys/socket.h>
#include <wire_protocol/wire_protocol.h>

#include <cstdint>
#include <memory>
#include <string>
#include <filesystem>

#include "../participant_util.h"
#include "console_task.h"
#include "thread_pool/task.h"

namespace participant_tasks {

/**
 * @class The persistent multicast receiver
 */
class MulticastReceiver : public thread_pool::Task {
 public:
  /**
   * @param console_task the console_task that this receiver will print to
   * @param log_location the location of the log file to log messages
   * @param port the port this receiver will listen on
   */
  MulticastReceiver(std::shared_ptr<ConsoleTask>  console_task,
                    std::filesystem::path log_location, int port);

  Status SetUp() override;
  Status RunAtomic() override;
  void CleanUp() override;

 private:
  /// Socket FD to listen on.
  int server_fd_ = -1;
  /// Socket FD to receive messages on.
  int messenger_fd_ = -1;

  int port_;
  std::filesystem::path log_location_;
  std::ofstream log_file_;
  std::shared_ptr<ConsoleTask> console_task_;

  /// buffer size for client.
  static constexpr size_t kBufferSize = 4096;

  /// buffer that stores serialized data to be received from the coordinator
  std::vector<uint8_t> incoming_msg_buf_{};

  /// parser for handling messages
  wire_protocol::MessageParser<pub_sub_messages::ForwardMulticast> parser_;
};
}  // namespace participant_tasks
#endif  // PROJECT3_MULTICAST_RECEIVER_H
