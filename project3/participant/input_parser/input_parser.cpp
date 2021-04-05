#include "input_parser.h"

namespace participant::input_parser {

InputParser::InputParser()
    : req_{},
      reg_msg_{},
      dereg_msg_{},
      discon_msg_{},
      recon_msg_{},
      msend_msg_{},
      is_valid_{},
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
      iss >> port_;
      break;
    case DISCON:
    case DEREG:
    case QUIT:
      break;
  }
}

bool InputParser::IsValid() { return is_valid_; }

google::protobuf::Message* InputParser::CreateReq() {
  switch (req_) {
    case REG:
      reg_msg_.set_port_number(port_);
      return &reg_msg_;
    case DEREG:
      return &dereg_msg_;
    case DISCON:
      return &discon_msg_;
    case RECON:
      reg_msg_.set_port_number(port_);
      return &recon_msg_;
    case MSEND:
      msend_msg_.set_message(message_);
      return &msend_msg_;
    case QUIT:
      return nullptr;
  }
  return nullptr;
}
}  // namespace participant::input_parser
