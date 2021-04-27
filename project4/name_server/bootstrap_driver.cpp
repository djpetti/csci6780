#include "bootstrap_driver.h"

#include <fstream>

namespace nameserver {

BootstrapDriver::BootstrapDriver(const std::filesystem::path config_file)
    : pool_(std::make_shared<thread_pool::ThreadPool>()),
      console_task_(
          std::make_shared<nameserver::tasks::ConsoleTask>("bootstrap=> ")) {
  LoadConfig(config_file);
  bootstrap_task_ =
      std::make_shared<nameserver::tasks::BootstrapTask>(bootstrap_);
}

void BootstrapDriver::LoadConfig(const std::filesystem::path& config_loc) {
  std::ifstream conf_file(config_loc);
  if (conf_file.is_open()) {
    std::string id;
    std::getline(conf_file, id);
    std::string port;
    std::getline(conf_file, port);
    std::string kv_pair;
    std::string key;
    std::string value;
    std::unordered_map<uint, std::string> kvs;
    while (std::getline(conf_file, kv_pair)) {
      std::istringstream ss(kv_pair);
      ss >> key;
      ss >> value;
      kvs.insert(std::pair<uint, std::string>(std::stoi(key), value));
    }
    bootstrap_ = std::make_shared<nameserver::Bootstrap>(
        pool_, console_task_, std::stoi(port), kvs);
  } else {
    console_task_->SendConsole("No config or invalid config found!");
    running_ = false;
  }
}

void BootstrapDriver::Start() {
  pool_->AddTask(console_task_);
  pool_->AddTask(bootstrap_task_);

  while (running_) {
    std::string input;
    std::getline(std::cin, input);
    std::string command;
    std::string key;
    std::string value;
    std::stringstream ss(input);
    ss >> command;
    ss >> key;
    auto itr = bootstrap_cmds.find(command);
    if (itr == bootstrap_cmds.end()) {
      console_task_->SendConsole("Invalid command!");
      continue;
    }
    switch (itr->second) {
      case BootstrapCommand::LOOKUP:
        console_task_->SendConsole("Performing lookup...");
        bootstrap_->LookUp(std::stoi(key));
        break;
      case BootstrapCommand::INSERT:
        console_task_->SendConsole("Performing insert...");
        ss >> value;
        bootstrap_->Insert(std::stoi(key), value);
        break;
      case BootstrapCommand::DELETE:
        console_task_->SendConsole("Performing delete...");
        bootstrap_->Delete(std::stoi(key));
        break;
    }
  }
}

}  // namespace nameserver
