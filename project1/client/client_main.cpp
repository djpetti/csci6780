/**
 * @brief Entry point for the FTP client.
 */

#include "client.h"
#include <loguru.hpp>

int main(int argc, const char **argv) {
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
