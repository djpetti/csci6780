/**
 * @file Name Server Class
 */
#ifndef PROJECT4_NAMESERVER_H
#define PROJECT4_NAMESERVER_H

#include <filesystem>
#include <map>
#include <string>

#include "consistent_hash_msgs.pb.h"
#include "message_passing/client.h"
#include "message_passing/server.h"
#include "message_passing/types.h"
#include "tasks/console_task.h"

namespace nameserver {

/**
 * @class The basic nameserver
 */
class Nameserver {
 public:
  /**
   * @brief Initializes the nameserver.
   * @param pool the threadpool of this server
   * @param console_task the console task of this server
   * @param id The ID of this nameserver.
   * @param port the port to listen on
   * @param bootstrap hostname and port information.
   */
  Nameserver(std::shared_ptr<thread_pool::ThreadPool> pool,
             std::shared_ptr<nameserver::tasks::ConsoleTask> console_task,
             uint id, int port, message_passing::Endpoint bootstrap);

  virtual ~Nameserver() = default;

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

  /**
   * @brief Receive a message and handle the message with server_
   */
  virtual void ReceiveAndHandle();

 protected:
  /// The threadpool used by client, server
  std::shared_ptr<thread_pool::ThreadPool> threadpool_;

  /// The server object
  std::unique_ptr<message_passing::Server> server_;

  /// The client object
  std::unique_ptr<message_passing::Client> client_;

  /// The key-value pairs in this nameserver
  std::unordered_map<uint, std::string> pairs_;

  /// The console task to use in printing
  std::shared_ptr<nameserver::tasks::ConsoleTask> console_task_;

  /// Key bounds
  std::pair<uint, uint> bounds_;

  /// Successor nameserver
  message_passing::Endpoint successor_;
  uint successor_id_;

  /// Predecessor nameserver
  message_passing::Endpoint predecessor_;
  uint predecessor_id_;

  /// Bootstrap endpoint
  message_passing::Endpoint bootstrap_;

  /// Timeout used to receive messages
  constexpr static const auto kTimeout = std::chrono::milliseconds(100);

  /**
   * @brief Handles a NameserverMessage
   * @param msg the message
   */
  void HandleRequest(const consistent_hash_msgs::NameServerMessage &msg,
                     const message_passing::Endpoint source);

  void HandleRequest(const consistent_hash_msgs::LookUpResult &request);

 private:
  /**
   *
   * @param A NameServerMessage request
   */
  void HandleRequest(const consistent_hash_msgs::EntranceInformation &request);

  void HandleRequest(const consistent_hash_msgs::ExitInformation &request);

  void HandleRequest(
      const consistent_hash_msgs::UpdatePredecessorRequest &request,
      const message_passing::Endpoint &source);

  void HandleRequest(
      const consistent_hash_msgs::UpdateSuccessorRequest &request,
      const message_passing::Endpoint &source);

  void HandleRequest(const consistent_hash_msgs::InsertResult &request);

  void HandleRequest(const consistent_hash_msgs::DeleteResult &request);

  /// Id of this name server
  uint id_;

  /// Port of this name server
  int port_;
};
}  // namespace nameserver

#endif  // PROJECT4_NAMESERVER_H
