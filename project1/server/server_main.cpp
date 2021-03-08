/**
 * @brief Entry point for the FTP server.
 */

#include <cstdlib>
#include <iostream>

#include "server.h"

/**
 * @brief Prints a usage message and exits.
 * @param program_name The name of the executable.
 */
void PrintUsageAndExit(const char* program_name) {
  std::cout << "Usage:" << std::endl
            << "  " << program_name << " normal_port termination_port" << std::endl;
  exit(1);
}

int main(int argc, const char** argv) {
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
  server.FtpService(nPort,tPort);
}