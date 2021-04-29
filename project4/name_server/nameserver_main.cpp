/**
 * @file Main executable for Name Server
 */

#include <iostream>
#include <loguru.hpp>

#include "bootstrap_driver.h"
#include "nameserver_driver.h"

void start_nameserver(char* config) {
  auto namespace_driver = nameserver::NameserverDriver(config);
  namespace_driver.Start();
}

void start_bootstrap(char* config) {
  auto bootstrap_driver = nameserver::BootstrapDriver(config);
  bootstrap_driver.Start();
}

int main(int argc, char** argv) {
  char log_path[1024];

  // initialize participant logging
  loguru::init(argc, argv);
  loguru::add_file("nameserver_info.log", loguru::Append,
                   loguru::Verbosity_INFO);
  loguru::add_file("nameserver_error.log", loguru::Append,
                   loguru::Verbosity_ERROR);
  loguru::suggest_log_path("./logs", log_path, sizeof(log_path));
  // Turn off stderr logging for the client because it interferes with our CLI.
  loguru::g_stderr_verbosity = loguru::Verbosity_OFF;

  if (argc != 3) {
    std::cout << "\nIncorrect # of inputs. Name server type and configuration "
                 "file expected";
    return 1;
  }

  if (argv[1] == std::string("simple")) {
    start_nameserver(argv[2]);
  } else if (argv[1] == std::string("bootstrap")) {
    start_bootstrap(argv[2]);
  } else {
    std::cout
        << "\nInvalid name server type. Available types are: simple, bootstrap";
    return 1;
  }

  return 0;
}
