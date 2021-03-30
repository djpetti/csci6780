#include "input_parser.h"

namespace participant::input_parser {

InputParser::InputParser()
    : multi_id_{0}, is_valid_{true}, req_{REG}, message_{} {}

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
      iss >> multi_id_;
      break;
    case DISCON:
    case DEREG:
      break;
  }
}  // InputParser

bool InputParser::IsValid() { return is_valid_; }

google::protobuf::Message* InputParser::CreateReq() {
  switch (req_) {
    case REG:
      return CreateRegMsg();
    case DEREG:
      return CreateDeregMsg();
    case DISCON:
      return CreateDisconMsg();
    case RECON:
      return CreateReconMsg();
    case MSEND:
      return CreateMsendMsg();
  }
  return nullptr;
}
pub_sub_messages::Register* InputParser::CreateRegMsg() {
  return pub_sub_messages::Register().New();
}
pub_sub_messages::Deregister* InputParser::CreateDeregMsg() {
  return pub_sub_messages::Deregister().New();
}
pub_sub_messages::Disconnect* InputParser::CreateDisconMsg() {
  return pub_sub_messages::Disconnect().New();
}
pub_sub_messages::Reconnect* InputParser::CreateReconMsg() {
  return pub_sub_messages::Reconnect().New();
}
pub_sub_messages::SendMulticast* InputParser::CreateMsendMsg() {
  pub_sub_messages::SendMulticast* msg =
      pub_sub_messages::SendMulticast().New();
  msg->set_message(message_);
  return msg;
}
}  // namespace participant::input_parser
