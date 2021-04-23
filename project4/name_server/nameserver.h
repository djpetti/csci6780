/**
 * @file Name Server Class
 */
#ifndef PROJECT4_NAME_SERVER_NAMESERVER_H_
#define PROJECT4_NAME_SERVER_NAMESERVER_H_

#include <map>
#include <string>

namespace nameserver {

/**
 * @class The basic name server
 */
class Nameserver {
 public:
  /**
   * @param the config file of this name server
   */
  Nameserver(char* config);

  /**
   * Start the name server
   */
  void start();

 private:
  /// Bootstrap server IP
  std::string bootstrap_ip_;

  /// Bootstrap server port
  int bootstrap_port_;

  /// Command Types
  enum Commands { ENTER, EXIT };

  /// Commands
  const std::map<std::string, Commands> commands_ = {{"enter", ENTER},
                                                     {"exit", EXIT}};

 protected:
  /// The key-value pairs in this nameserver
  std::map<int, std::string> pairs;

  /// Key bounds
  std::pair<int, int> bounds;

  /// Port of this name server
  int port;

  /// Id of this name server
  int id;
};
}  // namespace nameserver

#endif  // PROJECT4_NAME_SERVER_NAMESERVER_H_
