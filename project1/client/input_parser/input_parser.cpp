#include "input_parser.h"

namespace client::input_parser {
using ftp_messages::ChangeDirRequest;
using ftp_messages::DeleteRequest;
using ftp_messages::GetRequest;
using ftp_messages::ListRequest;
using ftp_messages::MakeDirRequest;
using ftp_messages::PutRequest;
using ftp_messages::PwdRequest;
using ftp_messages::QuitRequest;
using ftp_messages::Request;

InputParser::InputParser(std::string &cmd) {
  fn_ = "";
  dn_ = "";

  // iterate through each word in input
  // the validity of this code relies on correct input syntax
  std::istringstream iss(cmd);
  do {
    std::string word;
    server::file_handler::FileHandler fh;
    iss >> word;
    for (int i = 0; i < (int)commands_.size(); i++) {
      // determine which command, extract further information if necessary
      if (!word.compare(commands_[i])) {
        switch (i) {
          case 0:
            iss >> fn_;
            req_ = GETF;
            break;
          case 1:
            iss >> this->fn_;
            req_ = PUTF;
            contents_ = fh.Get(fn_);
            break;
          case 2:
            iss >> fn_;
            req_ = DEL;
            break;
          case 3:
            req_ = LS;
            break;
          case 4:
            iss >> dn_;
            req_ = CD;
            break;
          case 5:
            iss >> dn_;
            req_ = MKDIR;
            break;
          case 6:
            req_ = PWD;
            break;
          case 7:
            req_ = QUIT;
            break;
        }
      }
    }
  } while (iss);
}  // InputParser

std::string InputParser::GetFilename() { return fn_; }
Request InputParser::CreateGetReq() {
  Request request;
  request.mutable_get()->set_filename(fn_);
  return request;
}

Request InputParser::CreateReq() {
    Request request;
    switch (req_) {
        case GETF:
            return CreateGetReq();

        case PUTF:
            return CreatePutReq();

        case DEL:
            return CreateDelReq();

        case LS:
            return CreateListReq();

        case CD:
            return CreateCDReq();

        case MKDIR:
            return CreateMkdirReq();

        case PWD:
            return CreatePwdReq();

        case QUIT:
            return CreateQuitReq();
    }
    return (request);
}
Request InputParser::CreatePutReq() {
  Request request;
  request.mutable_put()->set_filename(fn_);
  std::string c(contents_.begin(), contents_.end());
  request.mutable_put()->set_file_contents(c);
  return request;
}
Request InputParser::CreateDelReq() {
  Request request;
  request.mutable_delete_()->set_filename(fn_);
  return request;
}
Request InputParser::CreateListReq() {
  Request request;
  request.mutable_list()->Clear();
  return request;
}
Request InputParser::CreateCDReq() {
  Request request;

  // compare returns 0 on equal strings, hence '!'
  if (!dn_.compare("..")) {
    request.mutable_change_dir()->set_go_up(true);
  } else {
      request.mutable_change_dir()->set_go_up(false);
  }
  request.mutable_change_dir()->set_dir_name(dn_);
  return request;
}
Request InputParser::CreateMkdirReq() {
  Request request;
  request.mutable_make_dir()->set_dir_name(dn_);
  return request;
}
Request InputParser::CreatePwdReq() {
  Request request;
  request.mutable_pwd();
  return request;
}
Request InputParser::CreateQuitReq() {
  Request request;
  request.mutable_quit();
  return request;
}
}  // namespace client::input_parser
