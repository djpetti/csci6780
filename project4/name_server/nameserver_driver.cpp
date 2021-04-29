#include "nameserver_driver.h"

#include <fstream>

namespace nameserver {

NameserverDriver::NameserverDriver(std::filesystem::path config_file)
    : pool_(std::make_shared<thread_pool::ThreadPool>()),
      console_task_(
          std::make_shared<nameserver::tasks::ConsoleTask>("nameserver=> ")) {
  LoadConfig(config_file);
  nameserver_task_ =
      std::make_shared<nameserver::tasks::NameserverTask>(nameserver_);
}

void NameserverDriver::LoadConfig(const std::filesystem::path& config_loc) {
  std::ifstream conf_file(config_loc);
  if (conf_file.is_open()) {
    std::string id;
    std::getline(conf_file, id);
    std::string port;
    std::getline(conf_file, port);
    std::string bootstrap_info;
    std::getline(conf_file, bootstrap_info);
    std::istringstream ss(bootstrap_info);
    message_passing::Endpoint bootstrap;
    ss >> bootstrap.hostname;
    ss >> bootstrap.port;
    nameserver_ = std::make_shared<nameserver::Nameserver>(
        pool_, console_task_, std::stoi(id), std::stoi(port), bootstrap);
  } else {
    console_task_->SendConsole("No config or invalid config found!");
    running_ = false;
  }
}

void NameserverDriver::Start() {
  pool_->AddTask(console_task_);
  pool_->AddTask(nameserver_task_);

  while (running_) {
    std::string input;
    std::getline(std::cin, input);
    auto itr = nameserver_cmds.find(input);
    if (itr == nameserver_cmds.end()) {
      console_task_->SendConsole("Invalid command!");
      continue;
    }
    switch (itr->second) {
      case NameserverCommand::ENTER:
        console_task_->SendConsole("Entering...");
        nameserver_->Enter();
        break;
      case NameserverCommand::EXIT:
        console_task_->SendConsole("Exiting...");
        nameserver_->Exit();
        break;
    }
  }
}

}  // namespace nameserver
