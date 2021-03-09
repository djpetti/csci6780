/**
 * @brief Entry point for the FTP server.
 */

#include <cstdlib>
#include <iostream>
#include <loguru.cpp>

#include "server.h"

/**
 * @brief Prints a usage message and exits.
 * @param program_name The name of the executable.
 */
void PrintUsageAndExit(const char *program_name) {
    LOG_F(INFO, "Incorrect program usage %s normal_port termination_port", program_name);
    exit(1);
}

int main(int argc, const char **argv) {
    char log_path[PATH_MAX];

    // initialize server logging
    loguru::init(argc,(char**) argv);
    loguru::add_file("server.log",loguru::Append, loguru::Verbosity_MAX);
    loguru::add_file("error.log", loguru::Append, loguru::Verbosity_ERROR);
    loguru::add_file("latest_error.log",loguru::Truncate, loguru::Verbosity_ERROR);
    loguru::suggest_log_path("../loguru/", log_path, sizeof(log_path));
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