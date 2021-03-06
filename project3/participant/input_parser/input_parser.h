/**
 * @file Handles input parsing for the participant in Project 3
 */

#ifndef PROJECT3_INPUT_PARSER_H
#define PROJECT3_INPUT_PARSER_H

#include <pub_sub_messages.pb.h>

#include <sstream>
#include <string>

#include "pub_sub_messages.pb.h"

namespace participant::input_parser {

/**
 * @brief Assists in mapping input commands for FTP client to corresponding
 * protobuf request.
 *
 */
class InputParser {
 public:
  enum MsgType { REG, DEREG, DISCON, RECON, MSEND, QUIT };

  /**
   * @brief Initializes relevant info about this input command
   * @param cmd the user input
   */
  InputParser();

  /**
   * @return whether or not the root command was valid
   */
  bool IsValid() const;

  /**
   * @param cmd the user input
   */
  void Parse(std::string& cmd);

  /**
   * creates the request to be sent
   * @return
   */
  const pub_sub_messages::CoordinatorMessage* CreateReq();

  /// request type
  MsgType req_{REG};

  /// request port, if provided
  int port_;

  /// participant identifier
  int participant_id_;

 private:
  const std::map<std::string, MsgType> commands_ = {
      {"register", REG},    {"deregister", DEREG}, {"disconnect", DISCON},
      {"reconnect", RECON}, {"msend", MSEND},      {"quit", QUIT}};

  /// message having been parsed from input
  pub_sub_messages::CoordinatorMessage coordinator_msg_{};

  /// information to be extracted from input
  bool is_valid_{true};
  std::string message_;
};
}  // namespace participant::input_parser
#endif  // PROJECT3_INPUT_PARSER_H
