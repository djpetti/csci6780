/**
 * @file Bootstrap class
 */
#ifndef PROJECT4_BOOTSTRAP_H
#define PROJECT4_BOOTSTRAP_H

#include <filesystem>

#include "nameserver.h"

namespace nameserver {

/**
 * @class Bootstrap server
 */
class Bootstrap : public Nameserver {
 public:
  /**
   * @brief Initializes
   * @param the config file of this bootstrap
   */
  explicit Bootstrap(const std::filesystem::path config_file);

  /**
   * @param Generic BootstrapMessage request
   */
  void HandleRequest(const consistent_hash_msgs::BootstrapMessage &request);

  /**
   * @param A BootstrapMessage request
   */
  void HandleRequest(const consistent_hash_msgs::EntranceRequest &request);

};
}  // namespace nameserver
#endif  // PROJECT4_BOOTSTRAP_H
