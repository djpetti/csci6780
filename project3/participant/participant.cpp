#include "participant.h"

#include <iostream>

#include <loguru.hpp>

namespace participant {
Participant::Participant(const std::filesystem::path& config_loc)
    : console_task_(
          std::make_shared<participant_tasks::ConsoleTask>("participant=> ")) {
  LoadConfig(config_loc);
}
void Participant::Start() {
  pool_.AddTask(console_task_);

  while (running_) {
    std::string input;
    std::getline(std::cin, input);
    input_parser_.Parse(input);
    // Validate user input.
    if (!input_parser_.IsValid()) {
      console_task_->SendConsole("Invalid command!");
      continue;
    }
    // If user requested to quit, exit participant peacefully
    if (input_parser_.req_ == input_parser::InputParser::QUIT) {
      std::cout << "Bye!" << std::endl;
      running_ = false;
      continue;
    }

    // Else, open connection and send the request
    int participant_fd = ConnectAndSend(*input_parser_.CreateReq());
    if (participant_fd == -1) {
      std::cout << "Couldn't instantiate a connection with coordinator."
                << std::endl;
      running_ = false;
      continue;
    }

    switch (input_parser_.req_) {
      case input_parser::InputParser::REG: {
        WaitForMessage(participant_fd);
        pub_sub_messages::RegistrationResponse response;
        parser_.GetMessage(&response);
        input_parser_.participant_id_ = response.participant_id();
      }
      case input_parser::InputParser::RECON: {
        if (connected_) {
          console_task_->SendConsole("Already connected!");
          break;
        }
        multicast_receiver_ =
            std::make_shared<participant_tasks::MulticastReceiver>(
                console_task_, log_location_, input_parser_.port_);
        pool_.AddTask(multicast_receiver_);
        connected_ = true;
        std::string statement =
            input_parser_.req_ == input_parser::InputParser::REG ? "Registered"
                                                                 : "Connected";
        console_task_->SendConsole(statement + ", listening on " +
                                   std::to_string(input_parser_.port_));
        break;
      }
      case input_parser::InputParser::DEREG:
      case input_parser::InputParser::DISCON: {
        if (!connected_) {
          console_task_->SendConsole("Nothing to disconnect from!");
          break;
        }
        pool_.CancelTask(multicast_receiver_);
        connected_ = false;
        std::string statement =
            input_parser_.req_ == input_parser::InputParser::DEREG
                ? "Deregistered!"
                : "Disconnected!";
        console_task_->SendConsole(statement);
        break;
      }
      case input_parser::InputParser::MSEND:
      default:
        break;
    }
    // Close participant connection
    close(participant_fd);
  }
}

void Participant::WaitForMessage(int fd_) {
  parser_.ResetParser();
  std::vector<uint8_t> outgoing_msg_buf{};
  int buf_size = 4096;
  while (!parser_.HasCompleteMessage()) {
    outgoing_msg_buf.resize(buf_size);
    const auto bytes_read =
        participant_util::ReceiveForever(fd_, outgoing_msg_buf.data(), buf_size, 0);
    if (bytes_read <= 0) {
      break;
    }
    outgoing_msg_buf.resize(bytes_read);
    parser_.AddNewData(outgoing_msg_buf);
  }
  outgoing_msg_buf.clear();
}

int Participant::ConnectAndSend(const google::protobuf::Message& msg) {
  int participant_fd = participant_util::SetUpSocket(
      participant_util::MakeAddress(coordinator_port_), coordinator_ip_);
  std::vector<uint8_t> outgoing_buf;
  wire_protocol::Serialize(msg, &outgoing_buf);
  if (participant_util::SendForever(participant_fd, outgoing_buf.data(),
                                    outgoing_buf.size(), 0) < 0) {
    LOG_S(0) << "Failed to send request";
    return -1;
  }
  return participant_fd;
}

void Participant::LoadConfig(const std::filesystem::path& config_loc) {
  std::ifstream conf_file(config_loc);
  if (conf_file.is_open()) {
    std::string line;
    std::getline(conf_file, line);
    log_location_ = line;
    std::getline(conf_file, line);
    std::stringstream ss(line);
    ss >> coordinator_ip_;
    ss >> coordinator_port_;
  } else {
    std::cout << "No config or invalid config found!" << std::endl;
    running_ = false;
  }
}
}  // namespace participant
