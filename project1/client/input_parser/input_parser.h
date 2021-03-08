#ifndef PROJECT1_INPUT_PARSER_H
#define PROJECT1_INPUT_PARSER_H

#include <ftp_messages.pb.h>

#include <array>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "../../server/file_handler/file_handler.h"
#include "../../wire_protocol/wire_protocol.h"
#include "ftp_messages.pb.h"

namespace client::input_parser {

/**
 * @brief Assists in mapping input commands for FTP client to corresponding
 * protobuf request.
 *
 */
class InputParser {
 public:
  enum ReqType { GETF, PUTF, DEL, LS, CD, MKDIR, PWD, QUIT, TERMINATE };

  /**
   * @brief Initializes relevant info about this input command
   * @param cmd the user input
   * @note assumes correct syntax
   */
  InputParser(std::string& cmd);

  /**
   * @brief getter for filename
   * @return the filename
   */
  std::string GetFilename();

  /**
   * @return whether or not the root command was valid
   */
  bool IsValid();

  /**
   * @return whether or not the user requested to fork the command
   */
  bool IsForking();

  /**
   * @brief gets the contents of a file
   * @return file contents message
   */
  ftp_messages::FileContents GetContentsMessage();

  /**
   * creates the request to be sent
   * @return
   */
  ftp_messages::Request CreateReq();

 private:
  /**
   * creates a get request
   * @return the request
   */
  ftp_messages::Request CreateGetReq();

  /**
   * creates a put request
   * @return the request
   */
  ftp_messages::Request CreatePutReq();

  /**
   * creates a delete request
   * @return the request
   */
  ftp_messages::Request CreateDelReq();

  /**
   * creates a list request
   * @return the request
   */
  ftp_messages::Request CreateListReq();

  /**
   * creates a cd request
   * @return the request
   */
  ftp_messages::Request CreateCdReq();

  /**
   * creates a mkdir request
   * @return the request
   */
  ftp_messages::Request CreateMkdirReq();

  /**
   * creates a pwd request
   * @return the request
   */
  ftp_messages::Request CreatePwdReq();

  /**
   * creates a quit request
   * @return the request
   */
  ftp_messages::Request CreateQuitReq();

  /**
   * creates a terminate request
   * @return the request
   */
  ftp_messages::Request CreateTerminateReq();

  const std::map<std::string, ReqType> commands_ = {
      {"get", GETF}, {"put", PUTF},  {"delete", DEL},
      {"ls", LS},    {"cd", CD},     {"mkdir", MKDIR},
      {"pwd", PWD},  {"quit", QUIT}, {"terminate", TERMINATE}};

  // information to be extracted from input
  bool is_valid_;
  bool is_forking_;
  ReqType req_;
  std::string fn_;
  std::string dn_;
  std::string cid_;
  std::vector<uint8_t> contents_;
};
};      // namespace client::input_parser
#endif  // PROJECT1_INPUT_PARSER_H
