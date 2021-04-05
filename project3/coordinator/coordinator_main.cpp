#include <string>
#include <fstream>
void LoadConfig(const std::string& config_loc) {
  std::ifstream conf_file(config_loc);
  if (conf_file.is_open()) {
    std::string line;
    std::getline(conf_file, line);
    log_location_ = line;
    std::getline(conf_file, line);
    std::stringstream ss(line);
    ss >> coordinator_ip_;
    ss >> coordinator_port_;
  } else {
    std::cout << "No config or invalid config found!" << std::endl;
    running_ = false;
  }
}
int main(int argc, char**argv) {

  return 0;
}
