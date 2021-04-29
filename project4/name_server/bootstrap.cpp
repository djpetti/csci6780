#include "bootstrap.h"

#include "loguru.hpp"

namespace nameserver {
Bootstrap::Bootstrap(
    std::shared_ptr<thread_pool::ThreadPool> pool,
    std::shared_ptr<nameserver::tasks::ConsoleTask> console_task, int port,
    std::unordered_map<uint, std::string> kvs)
    : Nameserver(pool, std::move(console_task), 0, port,
                 {"127.0.0.1", static_cast<uint16_t>(port)}) {
  pairs_ = std::move(kvs);
  bounds_ = {0, 1023};
  successor_ = bootstrap_;
}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::NameServerMessage& request,
    message_passing::Endpoint source) {
  if (request.has_entrance_request()) {
    LOG_F(INFO, "Bootstrap received EntranceRequest message from node %s",
          source.hostname.c_str());
    HandleRequest(request.entrance_request(), source);
  } else {
    Nameserver::HandleRequest(request, source);
  }
}

void Bootstrap::ReceiveAndHandle() {
  message_passing::Endpoint endpoint;
  consistent_hash_msgs::NameServerMessage message;
  if (server_->Receive(kTimeout, &message, &endpoint)) {
    HandleRequest(message, endpoint);
  }
}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::EntranceRequest& request,
    const message_passing::Endpoint& source) {
  consistent_hash_msgs::EntranceInformation info;
  entering_nameserver_ = source;
  if (successor_ == bootstrap_) {
    // ring is empty
    // Set entrance info with the bootstrap information.
    info.mutable_predecessor_info()->set_id(0);
    info.mutable_predecessor_info()->set_ip(bootstrap_.hostname);
    info.mutable_predecessor_info()->set_port(bootstrap_.port);

    info.mutable_successor_info()->CopyFrom(info.predecessor_info());

    if (!server_->SendAsync(info, source)) {
      LOG_S(ERROR) << "Failed to send EntranceInformation message.";
    }
  } else {
    // send message to be rippled.
    info.set_id(request.id());

    if (!server_->SendAsync(info, successor_)) {
      LOG_S(ERROR) << "Failed to send EntranceInformation message.";
    }
  }
}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::EntranceInformation& request) {
  consistent_hash_msgs::EntranceInformation req = request;
  consistent_hash_msgs::NameServerMessage message;
  message.mutable_entrance_info()->CopyFrom(req);
  if (req.successor_info().IsInitialized() &&
      !req.predecessor_info().IsInitialized()) {
    // the entering server will now be the last in the ring.
    message.mutable_entrance_info()->mutable_predecessor_info()->set_id(
        predecessor_id_);
    message.mutable_entrance_info()->mutable_predecessor_info()->set_ip(
        predecessor_.hostname);
    message.mutable_entrance_info()->mutable_predecessor_info()->set_port(
        predecessor_.port);

  } else if (!request.successor_info().IsInitialized() &&
             request.predecessor_info().IsInitialized()) {
    // the entering server will now be the first in the ring.
    message.mutable_entrance_info()->mutable_successor_info()->set_port(
        successor_.port);
    message.mutable_entrance_info()->mutable_successor_info()->set_ip(
        successor_.hostname);
    message.mutable_entrance_info()->mutable_successor_info()->set_id(
        successor_id_);

  } else if (!(req.mutable_predecessor_info()->IsInitialized() ||
               req.mutable_successor_info()->IsInitialized())) {
    LOG_F(FATAL,
          "EntranceInformation message is uninitialized after returning to the "
          "bootstrap.");
    return;
  }
  if (!server_->SendAsync(message, entering_nameserver_)) {
    LOG_F(ERROR, "Request failed to send.");
    // error
  } else {
    LOG_F(INFO, "Bootstrap sent InsertResult to successor $%i", successor_id_);
  }
}

void Bootstrap::LookUp(uint key) {
  // key-val resides in this nameserver
  const auto itr = pairs_.find(key);
  if (itr != pairs_.end()) {
    // key-val pair found , now print
    console_task_->SendConsole(std::string("Value: ").append(itr->second));
    console_task_->SendConsole("Contacted: 0");
  } else if (successor_ == bootstrap_) {
    console_task_->SendConsole("Key not found.");
  } else {
    LOG_F(INFO, "Initiating LookUp procedure.");
    consistent_hash_msgs::NameServerMessage message;
    message.mutable_look_up_result()->set_id(0);
    message.mutable_look_up_result()->set_key(key);
    message_passing::Client client =
        message_passing::Client(threadpool_, successor_);
    if (client.Send(message) < 0) {
      // error
      LOG_F(ERROR, "LookUpResult failed to send.");
    } else {
      LOG_F(INFO, "Bootstrap sent LookUpResult to successor $%i",
            successor_id_);
    }
  }
}

void Bootstrap::Insert(uint key, const std::string& val) {
  if (key >= bounds_.first && key <= bounds_.second) {
    // key-val should be inserted in this bootstrap
    std::pair<int, std::string> pair(key, val);
    pairs_.insert(pair);
    console_task_->SendConsole("Inserted into: 0");
    console_task_->SendConsole("Contacted: 0");
  } else if (successor_ == bootstrap_) {
    console_task_->SendConsole("Key out of range.");
  } else {
    consistent_hash_msgs::NameServerMessage message;
    message.mutable_insert_result()->set_id(0);
    message.mutable_insert_result()->set_key(key);
    message.mutable_insert_result()->set_value(val);

    message_passing::Client client =
        message_passing::Client(threadpool_, successor_);
    if (!client.SendAsync(message)) {
      // error
      LOG_F(ERROR, "Request failed to send.");
    } else {
      LOG_F(INFO, "Bootstrap sent InsertResult to successor $%i",
            successor_id_);
    }
  }
}

void Bootstrap::Delete(uint key) {
  // key-val resides in this nameserver
  auto itr = pairs_.find(key);
  if (itr != pairs_.end()) {
    // key-val pair found , now delete
    pairs_.erase(itr);
    console_task_->SendConsole("Contacted: 0");
  } else if (successor_ == bootstrap_) {
    console_task_->SendConsole("Key not found.");
  } else {
    consistent_hash_msgs::NameServerMessage message;
    message.mutable_delete_result()->set_delete_success(false);
    message.mutable_delete_result()->set_key(key);

    message_passing::Client client =
        message_passing::Client(threadpool_, successor_);
    if (!client.SendAsync(message)) {
      // error
      LOG_F(ERROR, "Request failed to send.");
    } else {
      LOG_F(INFO, "Bootstrap Sent DeleteResult to successor $%i",
            successor_id_);
    }
  }
}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::LookUpResult& request) {
  if (request.id() == 0) {
    // the id is 0 and the bootstrap did not insert.
    console_task_->SendConsole("Key not found.");
  } else {
    console_task_->SendConsole(std::string("Value: ").append(request.value()));
    PrintContacted(request.server_ids());
  }
}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::InsertResult& request) {
  if (request.id() == 0) {
    // the id is 0 and the bootstrap did not insert.
    console_task_->SendConsole("Key out of range.");
  } else {
    console_task_->SendConsole(
        std::string("Inserted into: ").append(std::to_string(request.id())));
    PrintContacted(request.server_ids());
  }
}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::DeleteResult& request) {
  if (!request.delete_success()) {
    console_task_->SendConsole("Key not found.");
  } else {
    console_task_->SendConsole("Successful deletion.");
    PrintContacted(request.server_ids());
  }
}
void Bootstrap::PrintContacted(
    google::protobuf::RepeatedField<google::protobuf::uint32> server_ids) {
  std::string contacted = "Contacted: ";
  for (auto server_id : server_ids) {
    contacted.append(std::to_string(server_id));
    contacted.append(", ");  /// TODO Minor: this adds a trailing comma
  }
  console_task_->SendConsole(contacted);
}

}  // namespace nameserver
