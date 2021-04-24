#include "bootstrap_driver.h"

namespace nameserver {

BootstrapDriver::BootstrapDriver(const std::string& config_file)
    : console_task_(
          std::make_shared<nameserver_tasks::ConsoleTask>("bootstrap=> ")),
      bootstrap_task_(std::make_shared<nameserver_tasks::BootstrapTask>(config_file)) {}

[[noreturn]] void BootstrapDriver::Start() {
  pool_.AddTask(console_task_);
  pool_.AddTask(bootstrap_task_);

  while (true) {
    std::string input;
    std::getline(std::cin, input);
    auto itr = bootstrap_cmds.find(input);
    if (itr == bootstrap_cmds.end()) {
      console_task_->SendConsole("Invalid command!");
      continue;
    }
    switch (itr->second) {
      case LOOKUP:
        console_task_->SendConsole("lookup!...");
        /// TODO Pass to bootstrap_task the lookup with extra
        break;
      case INSERT:
        console_task_->SendConsole("insert!...");
        /// TODO Pass to bootstrap_task the insert with extra
        break;
      case DELETE:
        console_task_->SendConsole("delete!...");
        /// TODO Pass to bootstrap_task the delete with extra
        break;
    }
  }
}

}  // namespace nameserver