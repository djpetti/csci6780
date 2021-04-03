#include "input_parser.h"

namespace participant::input_parser {

InputParser::InputParser()
    : reg_msg_{},
      dereg_msg_{},
      discon_msg_{},
      recon_msg_{},
      msend_msg_{},
      is_valid_{true},
      req_{REG},
      message_{} {}

void InputParser::Parse(std::string& cmd) {
  std::istringstream iss(cmd);
  std::string word;
  iss >> word;
  auto itr = commands_.find(word);
  is_valid_ = itr != commands_.end();
  if (!is_valid_) {
    return;
  }
  req_ = itr->second;
  switch (req_) {
    case MSEND:
      std::getline(iss, message_);
      break;
    case REG:
    case RECON:
    case DISCON:
    case DEREG:
      break;
  }
}  // InputParser

bool InputParser::IsValid() { return is_valid_; }

google::protobuf::Message* InputParser::CreateReq() {
  switch (req_) {
    case REG:
      return &reg_msg_;
    case DEREG:
      return &dereg_msg_;
    case DISCON:
      return &discon_msg_;
    case RECON:
      return &recon_msg_;
    case MSEND:
      msend_msg_.set_message(message_);
      return &msend_msg_;
  }
  return nullptr;
}
}  // namespace participant::input_parser
