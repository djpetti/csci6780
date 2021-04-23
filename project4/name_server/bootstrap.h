/**
 * @file Bootstrap class
 */
#ifndef PROJECT4_NAME_SERVER_BOOTSTRAP_H_
#define PROJECT4_NAME_SERVER_BOOTSTRAP_H_

#include "nameserver.h"

namespace nameserver {

/**
 * @class Bootstrap server
 */
class Bootstrap : public Nameserver {
 public:
  /**
   * @param the config file of this bootstrap
   */
  Bootstrap(char* config);

 private:
  /// Command Types
  enum Commands { LOOKUP, INSERT, DELETE };

  /// Commands
  const std::map<std::string, Commands> commands_ = {
      {"lookup", LOOKUP}, {"insert", INSERT}, {"delete", DELETE}};
};
}  // namespace nameserver
#endif  // PROJECT4_NAME_SERVER_BOOTSTRAP_H_
