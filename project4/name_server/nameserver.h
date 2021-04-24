/**
 * @file Name Server Class
 */
#ifndef PROJECT4_NAMESERVER_H
#define PROJECT4_NAMESERVER_H

#include <consistent_hash_msgs.pb.h>
#include "../../common/message_passing/types.h"

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
  explicit Nameserver(const std::string& config_file);

  /**
   * @param Generic NameServerMessage request
   */
  void HandleRequest(const consistent_hash_msgs::NameServerMessage &request);

  /**
   * @param A NameServerMessage request
   */
  void HandleRequest(const consistent_hash_msgs::EntranceInformation &request);

  void HandleRequest(const consistent_hash_msgs::ExitInformation &request);

  void HandleRequest(const consistent_hash_msgs::UpdatePredecessorRequest &request);

  void HandleRequest(const consistent_hash_msgs::UpdateSuccessorRequest &request);

  void HandleRequest(const consistent_hash_msgs::UpdatePredecessorResponse &request);

  void HandleRequest(const consistent_hash_msgs::LookUpResult &request);

  void HandleRequest(const consistent_hash_msgs::InsertResult &request);

  void HandleRequest(const consistent_hash_msgs::DeleteResult &request);

  /**
   * @param predecessor Predecessor if true, else successor
   * @param request The request to forward
   */
  void ForwardRequest(bool to_predecessor, const consistent_hash_msgs::NameServerMessage &request);

 protected:
  /// The key-value pairs in this nameserver
  std::map<int, std::string> pairs;

  /// Key bounds
  std::pair<int, int> bounds;

  /// Predecessor nameserver
  message_passing::Endpoint predecessor;

  /// Successor nameserver
  message_passing::Endpoint successor;

  /// Bootstrap endpoint
  message_passing::Endpoint bootstrap_;

  /// Port of this name server
  int port;

  /// Id of this name server
  int id;
};
}  // namespace nameserver

#endif  // PROJECT4_NAMESERVER_H
