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
            << "  " << program_name << " listen_port" << std::endl;
  exit(1);
}

int main(int argc, const char** argv) {
  if (argc != 2) {
    PrintUsageAndExit(argv[0]);
  }

  const auto kPort = strtol(argv[1], nullptr, 10);
  if (kPort == 0) {
    // Invalid port number.
    PrintUsageAndExit(argv[0]);
  }

  // Create the server.
  server::Server server;
  server.Listen(kPort);
}