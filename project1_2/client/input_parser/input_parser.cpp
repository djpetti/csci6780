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
using ftp_messages::TerminateRequest;
using ftp_messages::Request;
using ftp_messages::FileContents;

InputParser::InputParser(std::string &cmd) {
  fn_ = "";
  dn_ = "";
  is_valid_ = true;
  is_forking_ = false;

  std::istringstream iss(cmd);
  std::string word;
  server::file_handler::FileHandler fh;
  iss >> word;
  auto itr = commands_.find(word);
  if (itr == commands_.end()) {
    is_valid_ = false;
    return;
  }
  req_ = itr->second;
  switch(itr->second) {
    case PUTF:
      iss >> this->fn_;
      contents_ = fh.Get(fn_);
      break;
    case GETF:
    case DEL:
      iss >> fn_;
      break;
    case CD:
    case MKDIR:
      iss >> dn_;
      break;
    case TERMINATE:
      iss >> cid_;
      break;
    case LS:
    case PWD:
    case QUIT:
      break;
  }
  // assuming all else has processed properly,
  iss >> word;
  if (word == "&") {
    is_forking_ = true;
  }
}  // InputParser

std::string InputParser::GetFilename() { return fn_; }
bool InputParser::IsValid() { return is_valid_; }
bool InputParser::IsForking() { return is_forking_; }

FileContents InputParser::GetContentsMessage() {
  FileContents contents;
  std::string c(contents_.begin(), contents_.end());
  contents.set_contents(c);
  return contents;
}

Request InputParser::CreateReq() {
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
            return CreateCdReq();

        case MKDIR:
            return CreateMkdirReq();

        case PWD:
            return CreatePwdReq();

        case QUIT:
            return CreateQuitReq();

        case TERMINATE:
            return CreateTerminateReq();
    }

    return {};
}
Request InputParser::CreateGetReq() {
  Request request;
  request.mutable_get()->set_filename(fn_);
  return request;
}
Request InputParser::CreatePutReq() {
  Request request;
  request.mutable_put()->set_filename(fn_);
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
Request InputParser::CreateCdReq() {
  Request request;
  request.mutable_change_dir()->set_go_up(dn_ == "..");
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
Request InputParser::CreateTerminateReq() {
  Request request;
  request.mutable_terminate()->set_command_id(std::stoi(cid_));
  return request;
}
}  // namespace client::input_parser
