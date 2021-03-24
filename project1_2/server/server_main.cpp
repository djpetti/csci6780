/**
 * @brief Entry point for the FTP server.
 */

#include <cstdlib>
#include <loguru.hpp>

#include "server.h"

/**
 * @brief Prints a usage message and exits.
 * @param program_name The name of the executable.
 */
void PrintUsageAndExit(const char *program_name) {
  LOG_F(INFO, "Incorrect program usage %s normal_port termination_port",
        program_name);
  exit(1);
}

int main(int argc, char **argv) {
  char log_path[1024];

  // initialize server logging
  loguru::init(argc, argv);
  loguru::add_file("server_error.log", loguru::Append, loguru::Verbosity_ERROR);
  loguru::add_file("server_latest_error.log", loguru::Truncate,
                   loguru::Verbosity_ERROR);
  loguru::suggest_log_path("./logs", log_path, sizeof(log_path));
  loguru::add_file(log_path, loguru::FileMode::Truncate, loguru::Verbosity_MAX);

  if (argc != 3) {
    PrintUsageAndExit(argv[0]);
  }

  const auto nPort = strtol(argv[1], nullptr, 10);
  if (nPort == 0) {
    // Invalid port number.
    PrintUsageAndExit(argv[0]);
  }
  const auto tPort = strtol(argv[2], nullptr, 10);
  if (tPort == 0) {
    // Invalid port number.
    PrintUsageAndExit(argv[0]);
  }

  // Create the server.
  server::Server server;
  server.FtpService(nPort, tPort);
}