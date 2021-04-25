#include "nameserver_driver.h"

#include <utility>

namespace nameserver {

NameserverDriver::NameserverDriver(std::filesystem::path config_file)
    : console_task_(
          std::make_shared<nameserver::tasks::ConsoleTask>("nameserver=> ")),
      config_file_(std::move(config_file)) {}

[[noreturn]] void NameserverDriver::Start() {
  pool_.AddTask(console_task_);

  while (true) {
    std::string input;
    std::getline(std::cin, input);
    auto itr = nameserver_cmds.find(input);
    if (itr == nameserver_cmds.end()) {
      console_task_->SendConsole("Invalid command!");
      continue;
    }
    switch (itr->second) {
      case NameserverCommand::ENTER:
        console_task_->SendConsole("joining...");
        /// TODO Set up nameserver task
        break;
      case NameserverCommand::EXIT:
        console_task_->SendConsole("leaving!...");
        /// TODO Close down nameserver task
        break;
    }
  }
}

}  // namespace nameserver
