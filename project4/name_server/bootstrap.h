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

  ~Bootstrap() override = default;

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

 protected:
  /**
   * @brief Handles a general NameserverMessage
   * @param msg the message
   */
  void HandleRequest(const consistent_hash_msgs::NameServerMessage &msg,
                     message_passing::Endpoint source) override;

  /// Specializations for all common sub-messages.
  void HandleRequest(
      const consistent_hash_msgs::EntranceInformation &request) override;
  void HandleRequest(
      const consistent_hash_msgs::LookUpResult &request) override;
  void HandleRequest(
      const consistent_hash_msgs::InsertResult &request) override;
  void HandleRequest(
      const consistent_hash_msgs::DeleteResult &request) override;

 private:
  /**
   * @brief Using Protobuf, print to console_task_ a comma-separated, joined
   * list of contacted server_ids
   * @param server_ids
   */
  void PrintContacted(
      google::protobuf::RepeatedField<google::protobuf::uint32> server_ids);

  /// Handlers for bootstrap-specific messages.
  void HandleRequest(const consistent_hash_msgs::EntranceRequest &request,
                     const message_passing::Endpoint &source);

  /// The nameserver that wishes to enter the hash ring.
  message_passing::Endpoint entering_nameserver_;
};
}  // namespace nameserver
#endif  // PROJECT4_BOOTSTRAP_H
