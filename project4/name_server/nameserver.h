/**
 * @file Name Server Class
 */
#ifndef PROJECT4_NAMESERVER_H
#define PROJECT4_NAMESERVER_H

#include <consistent_hash_msgs.pb.h>

#include <filesystem>
#include <map>
#include <string>
#include "message_passing/server.h"
#include "message_passing/client.h"
#include "message_passing/types.h"

namespace nameserver {

/**
 * @class The basic nameserver
 */
class Nameserver {
 public:
  /**
   * @brief Initializes the nameserver based on config file.
   * @param the config file of this nameserver
   */
  explicit Nameserver(const std::filesystem::path config_file);

  /**
   * @brief Inserts this nameserver into the hash ring.
   * @return true on success false on failure
   */
   bool Enter();

   /**
    * @brief Deletes this nameserver from the hash ring.
    * @return true on success false on failure
    */
    void Exit();


 private:

  /**
   * Handles a generic NameServerMessage
   * @param Generic NameServerMessage request
   */
  void HandleRequest(const consistent_hash_msgs::NameServerMessage &request);

  /**
   *
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
   * @brief forwards the NameServerMessage and forwards it to the successor or
   * predecessor
   * @param predecessor Predecessor if true, else successor
   * @param request The request to forward
   */
  void ForwardRequest(bool to_predecessor, const consistent_hash_msgs::NameServerMessage &request);

  /// The server object
  std::unique_ptr<message_passing::Server> server_;

  /// The client object
  std::unique_ptr<message_passing::Client> client_;

  /// The threadpool used by the client
  std::shared_ptr<thread_pool::ThreadPool> server_threadpool_;

  /// The threadpool used by the server
  std::shared_ptr<thread_pool::ThreadPool> client_threadpool_;

  /// The key-value pairs in this nameserver
  std::unordered_map<int, std::string> pairs_;

  /// Key bounds
  std::pair<int, int> bounds_;

  /// Predecessor nameserver
  message_passing::Endpoint predecessor_;

  /// Successor nameserver
  message_passing::Endpoint successor_;

  /// Bootstrap endpoint
  message_passing::Endpoint bootstrap_;

  /// Had to comment these out because of "not used" warnings treated as errors
  /// Port of this name server
  //int port_;

  /// Id of this name server
  //int id_;
};
}  // namespace nameserver

#endif  // PROJECT4_NAMESERVER_H
