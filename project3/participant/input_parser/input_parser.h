/**
 * @file Handles input parsing for the participant in Project 3
 */

#ifndef PROJECT3_INPUT_PARSER_H
#define PROJECT3_INPUT_PARSER_H

#include <pub_sub_messages.pb.h>

#include <array>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "pub_sub_messages.pb.h"
#include "wire_protocol/wire_protocol.h"

namespace participant::input_parser {

/**
 * @brief Assists in mapping input commands for FTP client to corresponding
 * protobuf request.
 *
 */
class InputParser {
 public:
  enum MsgType { REG, DEREG, DISCON, RECON, MSEND };

  /**
   * @brief Initializes relevant info about this input command
   * @param cmd the user input
   */
  InputParser();

  /**
   * @return whether or not the root command was valid
   */
  bool IsValid();

  /**
   * @param cmd the user input
   */
  void Parse(std::string& cmd);

  /**
   * creates the request to be sent
   * @return
   */
  google::protobuf::Message* CreateReq();

 private:
  /**
   * creates a register message
   */
  pub_sub_messages::Register* CreateRegMsg();

  /**
   * creates a reregister message
   */
  pub_sub_messages::Deregister* CreateDeregMsg();

  /**
   * creates a disconnect message
   */
  pub_sub_messages::Disconnect* CreateDisconMsg();

  /**
   * creates a reconnect message
   */
  pub_sub_messages::Reconnect* CreateReconMsg();

  /**
   * creates a multicast send message
   */
  pub_sub_messages::SendMulticast* CreateMsendMsg();

  const std::map<std::string, MsgType> commands_ = {{"register", REG},
                                                    {"deregister", DEREG},
                                                    {"disconnect", DISCON},
                                                    {"reconnect", RECON},
                                                    {"msend", MSEND}};

  /// information to be extracted from input
  int multi_id_;
  bool is_valid_;
  MsgType req_;
  std::string message_;
};
};      // namespace participant::input_parser
#endif  // PROJECT3_INPUT_PARSER_H