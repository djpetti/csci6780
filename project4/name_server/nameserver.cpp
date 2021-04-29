#include "nameserver.h"

namespace nameserver {

Nameserver::Nameserver(
    std::shared_ptr<thread_pool::ThreadPool> pool,
    std::shared_ptr<nameserver::tasks::ConsoleTask> console_task, uint id,
    int port, message_passing::Endpoint bootstrap)
    : id_(id) {
  port_ = port;
  bootstrap_ = bootstrap;
  console_task_ = std::move(console_task);
  threadpool_ = pool;
  client_ = std::make_unique<message_passing::Client>(threadpool_, bootstrap_);
  server_ = std::make_unique<message_passing::Server>(threadpool_, port_);
}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::NameServerMessage& msg,
    const message_passing::Endpoint source) {
  consistent_hash_msgs::NameServerMessage message = msg;
  if (message.has_update_succ_req()) {
    LOG_F(INFO,
          "Nameserver #%i received UpdateSuccessorRequest message from node %s",
          id_, source.hostname.c_str());
    const auto req = message.update_succ_req();
    HandleRequest(req, source);
  } else if (message.has_delete_result()) {
    LOG_F(INFO, "Nameserver #%i received DeleteResult message from node %s",
          id_, source.hostname.c_str());
    const auto req = message.delete_result();
    HandleRequest(req);
  } else if (message.has_look_up_result()) {
    LOG_F(INFO, "Nameserver #%i received LookUpResult message from node %s",
          id_, source.hostname.c_str());
    const auto req = message.look_up_result();
    HandleRequest(req);
  } else if (message.has_insert_result()) {
    LOG_F(INFO, "Nameserver #%i received InsertResult message from node %s",
          id_, source.hostname.c_str());
    const auto req = message.insert_result();
    HandleRequest(req);
  } else if (message.has_entrance_info()) {
    LOG_F(INFO, "Nameserver #%i received EntranceInfo message from node %s",
          id_, source.hostname.c_str());
    const auto req = message.entrance_info();
    HandleRequest(req);
  } else if (message.has_exit_info()) {
    LOG_F(INFO, "Nameserver #%i received ExitInfo message from node %s", id_,
          source.hostname.c_str());
    HandleRequest(message.exit_info());
  } else if (message.has_update_pred_req()) {
    LOG_F(
        INFO,
        "Nameserver #%i received UpdatePredecessorRequest message from node %s",
        id_, source.hostname.c_str());
    const auto req = message.update_pred_req();
    HandleRequest(req, source);
  } else {
    LOG_F(ERROR, "Nameserver #%i received empty message from node %s", id_,
          source.hostname.c_str());
  }
}

bool Nameserver::Enter() {
  consistent_hash_msgs::BootstrapMessage bootstrap_message;
  consistent_hash_msgs::EntranceInformation entrance_info;
  consistent_hash_msgs::UpdatePredecessorResponse update_pred_res;

  LOG_F(INFO,
        "Nameserver #%i sending EntranceRequest message to bootstrap. Now "
        "waiting for an EntranceInfo from the server.",
        id_);
  // send entrance info request to bootstrap
  bootstrap_message.mutable_entrance_request()->set_id(id_);
  bootstrap_message.mutable_entrance_request()->set_port(port_);
  if (!client_->SendRequest(bootstrap_message, &entrance_info)) {
    LOG_F(ERROR, "No response received from the bootstrap.");
    return false;
  }

  // update successor & predecessor information
  successor_.port = entrance_info.successor_info().port();
  successor_.hostname = entrance_info.successor_info().ip();
  successor_id_ = entrance_info.successor_info().id();
  predecessor_.port = entrance_info.predecessor_info().port();
  predecessor_.hostname = entrance_info.predecessor_info().ip();
  predecessor_id_ = entrance_info.predecessor_info().id();

  // adjust hostnames if ring is empty since bootstrap cannot know it's own IP.
  if (predecessor_.hostname == "") {
    predecessor_.hostname = bootstrap_.hostname;
  }
  if (successor_.hostname == "") {
    successor_.hostname = bootstrap_.hostname;
  }
  LOG_S(INFO) << "Setting predecessor to " << predecessor_.hostname << ":"
              << predecessor_.port << " and successor to "
              << successor_.hostname << ":" << successor_.port << ".";

  LOG_F(INFO,
        "Nameserver #%i sending UpdatePredecessorRequest message to successor "
        "#%i. Waiting for an UpdatePredecessorResponse",
        id_, successor_id_);
  // tell this entering server's successor to update it's predecessor info
  message_passing::Client client =
      message_passing::Client(threadpool_, successor_);
  bootstrap_message.Clear();
  auto nameserver_message = bootstrap_message.mutable_name_server_message();
  nameserver_message->mutable_update_pred_req()
      ->mutable_predecessor_info()
      ->set_id(id_);
  nameserver_message->mutable_update_pred_req()
      ->mutable_predecessor_info()
      ->set_port(port_);
  if (successor_ == bootstrap_) {
    if (!client_->SendRequest(bootstrap_message, &update_pred_res)) {
      LOG_F(ERROR, "No UpdatePredecessorResponse received from the successor.");
      return false;
    }
  }
  else if (!client.SendRequest(bootstrap_message, &update_pred_res)) {
    LOG_F(ERROR, "No UpdatePredecessorResponse received from the successor.");
    return false;
  }

  // retrieve key-value information from the response
  bounds_.first = update_pred_res.lower_bounds();
  bounds_.second = update_pred_res.upper_bounds();
  LOG_S(INFO) << "Setting key range to [" << bounds_.first << ", "
              << bounds_.second << "].";
  for (int i = 0; i < update_pred_res.keys().size(); i++) {
    std::pair<int, std::string> pair(update_pred_res.keys().Get(i),
                                     update_pred_res.values().Get(i));
    pairs_.insert(pair);
  }
  LOG_F(
      INFO,
      "Nameserver #%i sending UpdateSuccessorRequest message to successor #%i",
      id_, successor_id_);
  // tell this entering server's predecessor to update it's successor info
  message_passing::Client pred_client =
      message_passing::Client(threadpool_, predecessor_);
  bootstrap_message.Clear();
  nameserver_message = bootstrap_message.mutable_name_server_message();
  nameserver_message->mutable_update_succ_req()
      ->mutable_successor_info()
      ->set_port(port_);
  nameserver_message->mutable_update_succ_req()
      ->mutable_successor_info()
      ->set_id(id_);
  if (pred_client.Send(bootstrap_message) < 0) {
    LOG_F(ERROR, "Failed to send UpdateSuccessorRequest to the predecessor.");
    return false;
  }

  return true;
}

void Nameserver::Exit() {
  consistent_hash_msgs::ExitInformation exit_info;
  consistent_hash_msgs::UpdateSuccessorRequest update_succ_req;
  LOG_F(INFO, "Nameserver #%i exiting.", id_);
  // prepare exit information
  exit_info.set_lower_bounds(bounds_.first);
  exit_info.set_upper_bounds(bounds_.second);
  for (const auto& pair : pairs_) {
    exit_info.add_keys(pair.first);
    exit_info.add_values(pair.second);
  }
  exit_info.mutable_predecessor_info()->set_port(predecessor_.port);
  exit_info.mutable_predecessor_info()->set_id(predecessor_id_);
  exit_info.mutable_predecessor_info()->set_ip(predecessor_.hostname);
  // send exit information to successor
  // so it can update it's key's and predecessor info
  message_passing::Client client =
      message_passing::Client(threadpool_, successor_);
  LOG_F(INFO, "Nameserver #%i sending ExitInfo message to successor #%i", id_,
        successor_id_);
  if (!client.SendAsync(exit_info)) {
    // error
    LOG_F(ERROR, "Request failed to send.");
  }
  // prepare update successor information
  update_succ_req.mutable_successor_info()->set_id(successor_id_);
  update_succ_req.mutable_successor_info()->set_port(successor_.port);
  update_succ_req.mutable_successor_info()->set_ip(successor_.hostname);

  LOG_F(INFO,
        "Nameserver #%i sending UpdateSuccessorRequest message to predecessor "
        "#%i ",
        id_, predecessor_id_);
  // tell predecessor to update it's successor info
  message_passing::Client pred_client =
      message_passing::Client(threadpool_, predecessor_);
  if (!pred_client.SendAsync(update_succ_req)) {
    // error
    LOG_F(ERROR, "Request failed to send.");
  }
}

void Nameserver::ReceiveAndHandle() {
  message_passing::Endpoint endpoint;
  consistent_hash_msgs::NameServerMessage ns_msg;
  if (server_->Receive(kTimeout, &ns_msg, &endpoint)) {
    HandleRequest(ns_msg, endpoint);
  }
}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::EntranceInformation& request) {
  consistent_hash_msgs::EntranceInformation req = request;
  if (req.id() > id_ && req.id() < successor_id_) {
    LOG_F(INFO,
          "Nameserver #%i will be the successor of entering name server #%i",
          id_, req.id());
    // this entering nameserver's to-be successor will be
    // this nameserver's successor.
    req.mutable_successor_info()->set_id(successor_id_);
    req.mutable_successor_info()->set_port(successor_.port);
    req.mutable_successor_info()->set_ip(successor_.hostname);
  } else if (request.id() < id_ && request.id() > predecessor_id_) {
    LOG_F(INFO,
          "Nameserver #%i will be the predecessor of entering name server #%i",
          id_, req.id());
    // this entering nameserver's predecessor will be
    // this nameserver's predecessor
    req.mutable_predecessor_info()->set_port(predecessor_.port);
    req.mutable_predecessor_info()->set_id(predecessor_id_);
    req.mutable_predecessor_info()->set_ip(predecessor_.hostname);
  }

  if (successor_ == bootstrap_ &&
      !req.mutable_successor_info()->IsInitialized() &&
      !req.mutable_predecessor_info()->IsInitialized()) {
    LOG_F(INFO, "Bootstrap will be the successor of entering name server #%i",
          req.id());
    // the entering nameserver will be the last successor in the hash ring.
    // the entering nameserver's to-be successor is the bootstrap server.
    // note that the bootstrap server will provide it's predecessor information
    // (which is this nameserver)
    req.mutable_successor_info()->set_id(successor_id_);
    req.mutable_successor_info()->set_port(successor_.port);
    req.mutable_successor_info()->set_ip(successor_.hostname);
  }
  // if the the request does not have predecessor info but has successor info by
  // the time it reaches back to bootstrap, we know the entering nameserver will
  // be the last in the ring. if the request does not have successor info but
  // has predecessor info by the time it reaches back to bootstrap, we know the
  // entering nameserver will be the first in the ring.

  // forward to successor
  message_passing::Client client =
      message_passing::Client(threadpool_, successor_);
  if (!client.SendAsync(req)) {
    LOG_F(ERROR,
          "Error sending EntranceInfo message from namserver #%i to successor "
          "#%i",
          id_, successor_id_);
  }
}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::ExitInformation& request) {
  // take over the exiting server's key-vals
  bounds_.second = request.upper_bounds();
  for (int i = 0; i < request.keys_size(); i++) {
    std::pair<int, std::string> pair(request.keys(i), request.values(i));
    pairs_.insert(pair);
  }

  // update predecessor info
  predecessor_.port = request.predecessor_info().port();
  predecessor_.hostname = request.predecessor_info().ip();
  predecessor_id_ = request.predecessor_info().id();
}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::UpdatePredecessorRequest& request,
    const message_passing::Endpoint& source) {
  // update predecessor info
  predecessor_.port = request.predecessor_info().port();
  predecessor_.hostname = source.hostname;
  predecessor_id_ = request.predecessor_info().id();

  // give lower half of it's key-val range to the entering nameserver
  consistent_hash_msgs::UpdatePredecessorResponse res;
  res.set_lower_bounds(bounds_.first);
  const uint kMiddleKey = (bounds_.second - bounds_.first) / 2;
  res.set_upper_bounds(bounds_.second - kMiddleKey);
  for (const auto& pair : pairs_) {
    if (pair.first <= kMiddleKey) {
      // this key-val goes to the entering nameserver
      res.add_keys(pair.first);
      res.add_values(pair.second);
      pairs_.erase(pair.first);
    }
  }
  bounds_.first += kMiddleKey + 1;

  LOG_F(INFO,
        "Nameserver #%i sending UpdatePredecessorResponse to name server %s",
        id_, source.hostname.c_str());
  if (!server_->SendAsync(res, source)) {
    // error
    LOG_F(ERROR, "Request failed to send.");
  }
}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::UpdateSuccessorRequest &request,
    const message_passing::Endpoint &source) {
  // update successor info
  successor_.hostname = request.successor_info().ip();
  if (successor_.hostname == "") {
    // an entering nameserver has sent this request.
    successor_.hostname = source.hostname;
  }
  successor_.port = request.successor_info().port();
  successor_id_ = request.successor_info().id();
}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::LookUpResult& request) {
  consistent_hash_msgs::LookUpResult req = request;
  if (req.key() <= bounds_.second && req.key() >= bounds_.first) {
    // key-val resides in this nameserver
    auto itr = pairs_.find(req.key());
    if (itr != pairs_.end()) {
      // value found
      req.set_value(itr->second);
      req.set_id(id_);
    }
  }
  // denote this server as contacted
  auto ids = req.server_ids();
  req.add_server_ids(id_);
  LOG_F(INFO, "Nameserver #%i sending a LookUpResult to successor #%i", id_,
        successor_id_);
  // forward LookUpResult to successor
  message_passing::Client client =
      message_passing::Client(threadpool_, successor_);
  if (!client.SendAsync(req)) {
    // error
    LOG_F(ERROR, "Request failed to send.");
  }
}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::InsertResult& request) {
  consistent_hash_msgs::InsertResult req = request;
  if (req.key() <= bounds_.second && req.key() >= bounds_.first) {
    // key-val should be inserted in this nameserver
    std::pair<int, std::string> pair(req.key(), req.value());
    pairs_.insert(pair);
    req.set_id(id_);
  }
  // denote this server as contacted
  auto ids = req.server_ids();
  req.add_server_ids(id_);

  LOG_F(INFO, "Nameserver #%i sending a InsertResult to successor #%i", id_,
        successor_id_);
  // forward LookUpResult to successor
  message_passing::Client client =
      message_passing::Client(threadpool_, successor_);
  if (!client.SendAsync(req)) {
    // error
    LOG_F(ERROR, "Request failed to send.");
  }
}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::DeleteResult& request) {
  consistent_hash_msgs::DeleteResult req = request;
  if (req.key() <= bounds_.second && req.key() >= bounds_.first) {
    // key-val resides in this nameserver
    auto itr = pairs_.find(req.key());
    if (itr != pairs_.end()) {
      // key-val pair found , now delete
      pairs_.erase(itr);
      req.set_delete_success(true);
    }
  }
  // denote this server as contacted
  auto ids = req.server_ids();
  req.add_server_ids(id_);

  LOG_F(INFO, "Nameserver #%i sending a DeleteResult to successor #%i", id_,
        successor_id_);
  // forward LookUpResult to successor
  message_passing::Client client =
      message_passing::Client(threadpool_, successor_);
  if (!client.SendAsync(req)) {
    // error
    LOG_F(ERROR, "Request failed to send.");
  }
}

}  // namespace nameserver
