#include "nameserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
namespace nameserver {

Nameserver::Nameserver(
    std::shared_ptr<thread_pool::ThreadPool> pool,
    std::shared_ptr<nameserver::tasks::ConsoleTask> console_task,
    const std::filesystem::path config_file) {
  console_task_ = console_task;
  threadpool_ = pool;
  client_ = std::make_unique<message_passing::Client>(threadpool_, bootstrap_);
}

std::string GetIp() {

}

bool Nameserver::Enter() {
  consistent_hash_msgs::EntranceRequest entrance_req;
  consistent_hash_msgs::EntranceInformation entrance_info;
  consistent_hash_msgs::UpdatePredecessorResponse update_pred_res;
  consistent_hash_msgs::UpdatePredecessorRequest update_pred_req;
  consistent_hash_msgs::UpdateSuccessorRequest update_succ_req;

  // send entrance info request to bootstrap
  if (!client_->SendRequest(entrance_req, &entrance_info)) return false;

  // update successor & predecessor information
  successor_.port = entrance_info.mutable_successor_info()->port();
  successor_.hostname = entrance_info.mutable_successor_info()->ip();
  predecessor_.port = entrance_info.mutable_predecessor_info()->port();
  predecessor_.hostname = entrance_info.mutable_predecessor_info()->ip();

  // tell this entering server's successor to update it's predecessor info
  client_ = std::make_unique<message_passing::Client>(threadpool_, successor_);
  update_pred_req.mutable_predecessor_info()->set_id(id_);
  update_pred_req.mutable_predecessor_info()->set_port(port_);
  if (!client_->SendRequest(update_pred_req, &update_pred_res)) return false;

  // retrieve key-value information from the response
  bounds_.first = update_pred_res.lower_bounds();
  bounds_.second = update_pred_res.upper_bounds();
  for (int i = 0; i < update_pred_res.mutable_keys()->size(); i++) {
    std::pair<int, std::string> pair(update_pred_res.mutable_keys()->at(i),
                                     update_pred_res.mutable_values()->at(i));
    pairs_.insert(pair);
  }
  // tell this entering server's predecessor to update it's successor info
  client_ =
      std::make_unique<message_passing::Client>(threadpool_, predecessor_);
  update_succ_req.mutable_successor_info()->set_port(port_);
  update_succ_req.mutable_successor_info()->set_id(id_);
  if (client_->Send(update_succ_req) < 0) return false;

  return true;
}

void Nameserver::Exit() {
  consistent_hash_msgs::ExitInformation exit_info;
  int count = 0;
  // prepare exit information
  exit_info.set_lower_bounds(bounds_.first);
  exit_info.set_upper_bounds(bounds_.second);
  for (std::pair<int, std::string> pair : pairs_) {
    exit_info.set_keys(count, pair.first);
    exit_info.set_values(count, pair.second);
    count++;
  }
  // send exit information to successor
  // successor will inform this server's predecessor to update it's successor
  client_ = std::make_unique<message_passing::Client>(threadpool_, successor_);
  if (client_->Send(exit_info) < 0) {
    // error
  }
}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::NameServerMessage& request) {
  HandleRequest(request);
}

void Nameserver::HandleRequest(
    consistent_hash_msgs::EntranceInformation& request) {
  if (request.id() == (id_ + 1)) {
    // this nameserver is the entering nameserver's predecessor
    request.mutable_predecessor_info()->set_id(id_);
    request.mutable_predecessor_info()->set_port(port_);
  } else if (request.id() == (id_ - 1)) {
    // this nameserver is the entering nameserver's to-be successor.
    // the entering nameserver's successor will be this name server's current successor
    request.mutable_successor_info()->set_port(successor_.port);
    request.mutable_successor_info()->set_ip(successor_.hostname);
  }
  if (successor_ == bootstrap_ && !request.successor_info().IsInitialized() &&
      !request.predecessor_info().IsInitialized()) {
    request.mutable_successor_info()->set_id(kBootstrapID);
    request.mutable_successor_info()->set_port(bootstrap_.port);
  }
}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::ExitInformation& request) {}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::UpdatePredecessorRequest& request) {}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::UpdateSuccessorRequest& request) {}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::UpdatePredecessorResponse& request) {}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::LookUpResult& request) {}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::InsertResult& request) {}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::DeleteResult& request) {}

}  // namespace nameserver
