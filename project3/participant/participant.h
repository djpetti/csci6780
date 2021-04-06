/**
 * @file Multicast Participant Class
 */
#ifndef PROJECT3_PARTICIPANT_H
#define PROJECT3_PARTICIPANT_H

#include <cstdint>
#include <filesystem>

#include "../../common/thread_pool/task.h"
#include "../../common/thread_pool/thread_pool.h"
#include "input_parser/input_parser.h"
#include "pub_sub_messages.pb.h"
#include "tasks/console_task.h"
#include "tasks/multicast_receiver_task.h"

namespace participant {

/**
 * @class Handles participant thread and input coordination
 */
class Participant {
 public:
  /**
   * @param config_loc the location of the config file
   */
  explicit Participant(const std::filesystem::path& config_loc);

  /**
   * Start the participant
   */
  void Start();

 private:
  /**
   * @param config_loc the config file location
   */
  void LoadConfig(const std::filesystem::path& config_loc);

  /**
   * @param msg the message to send to the coordinator
   * @return successfully sent
   */
  int ConnectAndSend(const google::protobuf::Message& msg);

  /**
   *
   */
   void WaitForMessage(int fd_);

  /// Pool and tasks
  thread_pool::ThreadPool pool_;
  std::shared_ptr<participant_tasks::ConsoleTask> console_task_;
  std::shared_ptr<participant_tasks::MulticastReceiver> multicast_receiver_;

  /// Config options
  std::filesystem::path log_location_;
  std::string coordinator_ip_;
  int coordinator_port_;

  /// The command parser
  input_parser::InputParser input_parser_;

  /// Registration response parser
  wire_protocol::MessageParser<pub_sub_messages::RegistrationResponse> parser_;

  /// Whether or not to continue running
  bool running_{true};

  /// If the user is currently connected to a group
  bool connected_{false};
};
}  // namespace participant

#endif  // PROJECT3_PARTICIPANT_H
