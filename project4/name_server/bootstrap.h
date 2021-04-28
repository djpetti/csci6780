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
   * @brief Initializes the bootstrap server from config file parameters.
   * @param the config file of this bootstrap
   */
  Bootstrap(std::shared_ptr<thread_pool::ThreadPool> pool,
            std::shared_ptr<nameserver::tasks::ConsoleTask> console_task,
            int port, std::unordered_map<uint, std::string> kvs);

  /**
   * Handles a generic BootstrapMessage
   * @param Generic BootstrapMessage request
   */
  void HandleRequest(const consistent_hash_msgs::BootstrapMessage &request,
                     message_passing::Endpoint source);

  /**
   * @brief Inserts a key-value pair in the ring.
   * @param key The integer key for the corresponding value
   * @param val The value
   */
  void Insert(uint key, const std::string &val);

  /**
   * @brief Deletes a key-value pair from the ring.
   * @param key The integer key for the corresponding value
   */
  void Delete(uint key);

  /**
   * @brief Looks up a value hosted in the ring.
   * @param key The integer key of the value to look up.
   */
  void LookUp(uint key);

  /**
   * @brief Receive a message and handle the message with server_
   */
  void ReceiveAndHandle() override;

 private:
  /**
   * @brief Handles an entrance request from an entering name server.
   * @param A BootstrapMessage request
   */
  void HandleRequest(const consistent_hash_msgs::EntranceRequest &request,
                     const message_passing::Endpoint source);

  /**
   * @param A NameServerMessage request
   */
  void HandleRequest(const consistent_hash_msgs::EntranceInformation &request);

  void HandleRequest(const consistent_hash_msgs::LookUpResult &request);

  void HandleRequest(const consistent_hash_msgs::InsertResult &request);

  void HandleRequest(const consistent_hash_msgs::DeleteResult &request);

  /**
   * @brief Using Protobuf, print to console_task_ a comma-separated, joined
   * list of contacted server_ids
   * @param server_ids
   */
  void PrintContacted(
      google::protobuf::RepeatedField<google::protobuf::uint32> server_ids);

  /// The nameserver that wishes to enter the hash ring.
  message_passing::Endpoint entering_nameserver_;
};
}  // namespace nameserver
#endif  // PROJECT4_BOOTSTRAP_H
