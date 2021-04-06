#include "input_parser.h"

namespace participant::input_parser {

InputParser::InputParser()
    : req_{},
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

bool InputParser::IsValid() const { return is_valid_; }

const pub_sub_messages::CoordinatorMessage* InputParser::CreateReq() {
  coordinator_msg_.Clear();

  switch (req_) {
    case REG:
      coordinator_msg_.mutable_register_()->set_port_number(port_);
      break;
    case DEREG:
      coordinator_msg_.mutable_deregister()->set_participant_id(participant_id_);
      break;
    case DISCON:
      coordinator_msg_.mutable_disconnect()->set_participant_id(participant_id_);
      break;
    case RECON:
      coordinator_msg_.mutable_reconnect()->set_participant_id(participant_id_);
      coordinator_msg_.mutable_reconnect()->set_port_number(port_);
      break;
    case MSEND:
      coordinator_msg_.mutable_send_multicast()->set_participant_id(participant_id_);
      coordinator_msg_.mutable_send_multicast()->set_message(message_);
      break;
    case QUIT:
      return nullptr;
  }
  return &coordinator_msg_;
}
}  // namespace participant::input_parser
