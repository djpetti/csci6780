/**
 * @brief Entry point for the FTP client.
 */

#include <loguru.hpp>

#include "client.h"

int main(int argc, char **argv) {
  char log_path[1024];

  // initialize client logging
  loguru::init(argc, argv);
  loguru::add_file("client_error.log", loguru::Append, loguru::Verbosity_ERROR);
  loguru::add_file("client_latest_error.log", loguru::Truncate,
                   loguru::Verbosity_ERROR);
  loguru::suggest_log_path("./logs", log_path, sizeof(log_path));
  loguru::add_file(log_path, loguru::FileMode::Truncate, loguru::Verbosity_MAX);
  // Turn off stderr logging for the client because it interferes with our CLI.
  loguru::g_stderr_verbosity = loguru::Verbosity_OFF;

  if (argc != 4) {
    std::cout << "\nIncorrect # of inputs. Server IP address, Command Port #, and Terminate Port # expected";
    return 1;
  }
  client::Client ftp_client;

  // second program argument should be the command port #
  const auto nPort = strtol(argv[2], nullptr, 10);

  // third program argument should be the terminate port #
  const auto tPort = strtol(argv[3], nullptr, 10);

  // first program argument should be the server IP address
  if (!ftp_client.Connect(argv[1], nPort, tPort)) {
    std::cout << "Failed to connect to " << argv[1] << ":" << nPort;
    return 1;
  }

  // spawn the shell
  ftp_client.FtpShell();
  return 0;
}
