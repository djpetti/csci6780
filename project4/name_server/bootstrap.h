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
            const std::filesystem::path config_file);

  /**
   * Handles a generic NameServerMessage
   * @param Generic NameServerMessage request
   */
  void HandleRequest(const google::protobuf::Message &request);

  /**
   * @brief Inserts a key-value pair in the ring.
   * @param key The integer key for the corresponding value
   * @param val The value
   */
  void Insert(int key, std::string val);

  /**
   * @brief Deletes a key-value pair from the ring.
   * @param key The integer key for the corresponding value
   * @param val The value
   */
  void Delete(int key, std::string val);

  /**
   * @brief Looks up a value hosted in the ring.
   * @param key The integer key of the value to look up.
   * @return The corresponding value. Empty string if value DNE
   */
  std::string LookUp(int key);

 private:
  /**
   * @brief Handles an entrance request from an entering name server.
   * @param A BootstrapMessage request
   */
  void HandleRequest(const consistent_hash_msgs::EntranceRequest &request);

  /**
   * @brief Initiates the insertion process for a joining name server.
   * @param server The joining server
   */
  void InitiateEntrance(const message_passing::Endpoint server);

  /**
   * @param A NameServerMessage request
   */
  void HandleRequest(const consistent_hash_msgs::EntranceInformation &request);

  void HandleRequest(const consistent_hash_msgs::ExitInformation &request);

  void HandleRequest(const consistent_hash_msgs::LookUpResult &request);

  void HandleRequest(const consistent_hash_msgs::InsertResult &request);

  void HandleRequest(const consistent_hash_msgs::DeleteResult &request);
};
}  // namespace nameserver
#endif  // PROJECT4_BOOTSTRAP_H
