/**
 * @file Main executable for the Coordinator.
 */
#include <string>
#include <fstream>
#include <loguru.hpp>
#include "coordinator_driver.h"
/**
 * @brief Helper function that extracts Coordinator parameters from a config file
 * @param config_loc The location of the config file.
 */
void LoadConfig(const std::string& config_loc) {
  LOG_F(INFO, "Loading config file %s", config_loc.c_str());
  std::ifstream conf_file(config_loc);
  int port;
  if (conf_file.is_open()) {
    std::string line;
    std::getline(conf_file, line);
    port = std::stoi(line);
    std::getline(conf_file, line);
    int time = std::stoi(line);

    auto driver = std::make_shared<coordinator::CoordinatorDriver>();
    driver->Start(port, coordinator::Duration ((long) time));
  } else {
    LOG_F(INFO,"Config file not found");
  }
}
int main(int argc, char**argv) {
  if (argc != 2) {
    LOG_F(ERROR, "Incorrect number of program arguments.");
    return 0;
  }
  LoadConfig(argv[1]);
  return 0;
}
