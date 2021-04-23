/**
 * @file Main executable for Name Server
 */

#include <loguru.hpp>

#include <iostream>

void start_nameserver(char* string) {

}

void start_bootstrap(char* string) {

}

int main(int argc, char **argv) {
  char log_path[1024];

  // initialize participant logging
  loguru::init(argc, argv);
  loguru::add_file("nameserver_error.log", loguru::Append,
                   loguru::Verbosity_ERROR);
  loguru::suggest_log_path("./logs", log_path, sizeof(log_path));
  // Turn off stderr logging for the client because it interferes with our CLI.
  loguru::g_stderr_verbosity = loguru::Verbosity_OFF;

  if (argc != 3) {
    std::cout << "\nIncorrect # of inputs. Name server type and configuration file expected";
    return 1;
  }

  if (argv[1] == std::string("simple")) {
    std::cout << "\nStarting name server...";
    start_nameserver(argv[2]);
  } else if (argv[1] == std::string("bootstrap")) {
    std::cout << "\nStarting bootstrap...";
    start_bootstrap(argv[2]);
  } else {
    std::cout << "\nInvalid name server type. Available types are: simple, bootstrap";
    return 1;
  }

  return 0;
}
