#include "bootstrap.h"

namespace nameserver {
Bootstrap::Bootstrap(
    std::shared_ptr<thread_pool::ThreadPool> pool,
    std::shared_ptr<nameserver::tasks::ConsoleTask> console_task, int port,
    std::unordered_map<uint, std::string> kvs)
    : Nameserver(pool, console_task, port,
                 {"127.0.0.1", static_cast<uint16_t>(port)}) {
  pairs_ = kvs;
  bounds_.first = 0;
  bounds_.second = 1023;
  successor_ = bootstrap_;
}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::BootstrapMessage& request,
    message_passing::Endpoint source) {
  if (request.has_entrance_request()) {
    HandleRequest(request.entrance_request(), source);
  } else if (request.has_name_server_message()) {
    /// FIXME if receiving a type of NameServerMessage, but listening for a
    /// BootstrapMessage, the BootstrapMessage message will have not an
    /// EntranceRequest nor an instance of NameServerMessage.
    auto msg = request.name_server_message();
    if (msg.has_delete_result()) {
      auto del_msg = msg.delete_result();
      HandleRequest(del_msg);
    } else if (msg.has_insert_result()) {
      auto ins_msg = msg.insert_result();
      HandleRequest(ins_msg);
    } else if (msg.has_look_up_result()) {
      auto lou_msg = msg.look_up_result();
      HandleRequest(lou_msg);
    } else if (msg.has_entrance_info()) {
      auto ent_msg = msg.entrance_info();
      HandleRequest(ent_msg);
    } else {
      Nameserver::HandleRequest(msg, source);
    }
  }
}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::EntranceRequest& request, const message_passing::Endpoint source) {
  consistent_hash_msgs::EntranceInformation entrance_info;
  entering_nameserver_ = source;
  if (successor_ == bootstrap_) {
    // ring is empty
    client_ = std::make_unique<message_passing::Client>(threadpool_, source);
    // send empty entrance info message.
    // the joining nameserver will know it's predecessor and successor are the bootstrap server.
    client_->Send(entrance_info);
  } else {
    // send entrance info message to be filled out
    entrance_info.set_id(request.id());
    client_ = std::make_unique<message_passing::Client>(threadpool_, successor_);
    client_->Send(entrance_info);
  }

}

void Bootstrap::HandleRequest(
    consistent_hash_msgs::EntranceInformation& request) {
  if (request.successor_info().IsInitialized() && !request.predecessor_info().IsInitialized()) {
    // the entering server will now be the last in the ring.
    request.mutable_predecessor_info()->set_id(predecessor_id_);
    request.mutable_predecessor_info()->set_ip(predecessor_.hostname);
    request.mutable_predecessor_info()->set_port(predecessor_.port);
  } else if (!request.successor_info().IsInitialized() && request.predecessor_info().IsInitialized()) {
    // the entering server will now be the first in the ring.
    request.mutable_successor_info()->set_port(successor_.port);
    request.mutable_successor_info()->set_ip(successor_.hostname);
    request.mutable_predecessor_info()->set_id(successor_id_);
  } else if (request.mutable_predecessor_info()->IsInitialized() || request.mutable_successor_info()->IsInitialized()) {
    // this should never happen.
    // error
    return;
  }
  client_ = std::make_unique<message_passing::Client>(threadpool_, entering_nameserver_);
  if (client_->Send(request) < 0 ) {
    // error
  }
}

void Bootstrap::LookUp(uint key) {
  // key-val resides in this nameserver
  auto itr = pairs_.find(key);
  if (itr != pairs_.end()) {
    // key-val pair found , now print
    console_task_->SendConsole(std::string("Value: ").append(itr->second));
    console_task_->SendConsole("Contacted: 0");
  } else if (successor_ == bootstrap_) {
    console_task_->SendConsole("Key not found.");
  } else {
    consistent_hash_msgs::LookUpResult lookup;
    lookup.set_id(0);
    lookup.set_key(key);
    client_ =
        std::make_unique<message_passing::Client>(threadpool_, successor_);
    if (client_->Send(lookup) < 0) {
      // error
    }
  }
}

void Bootstrap::Insert(uint key, std::string val) {
  if (key >= bounds_.first && key <= bounds_.second) {
    // key-val should be inserted in this bootstrap
    std::pair<int, std::string> pair(key, val);
    pairs_.insert(pair);
    console_task_->SendConsole("Inserted into: 0");
    console_task_->SendConsole("Contacted: 0");
  } else if (successor_ == bootstrap_) {
    console_task_->SendConsole("Key out of range.");
  } else {
    consistent_hash_msgs::InsertResult insert;
    insert.set_id(0);
    insert.set_key(0);
    client_ =
        std::make_unique<message_passing::Client>(threadpool_, successor_);
    if (client_->Send(insert) < 0) {
      // error
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
    consistent_hash_msgs::DeleteResult delete_r;
    delete_r.set_delete_success(false);
    delete_r.set_key(key);
    client_ =
        std::make_unique<message_passing::Client>(threadpool_, successor_);
    if (client_->Send(delete_r) < 0) {
      // error
    }
  }
}

void Bootstrap::HandleRequest(consistent_hash_msgs::LookUpResult& request) {
  // denote this server as contacted
  auto ids = request.server_ids();
  request.set_server_ids(ids.size(), 0);

  if (request.value().empty()) {
    // id 0 indicates bootstrap, except bootstrap never sends this request
    // fulfilled by 0
    console_task_->SendConsole("Key not found.");
  } else {
    console_task_->SendConsole(std::string("Value: ").append(request.value()));
    PrintContacted(request.server_ids());
  }
}

void Bootstrap::HandleRequest(consistent_hash_msgs::InsertResult& request) {
  // denote this server as contacted
  auto ids = request.server_ids();
  request.set_server_ids(ids.size(), 0);
  if (request.id() == 0) {
    // the id is 0 and the bootstrap did not insert.
    console_task_->SendConsole("Key out of range.");
  } else {
    console_task_->SendConsole(
        std::string("Inserted into: ").append(std::to_string(request.id())));
    PrintContacted(request.server_ids());
  }
}

void Bootstrap::HandleRequest(consistent_hash_msgs::DeleteResult& request) {
  // denote this server as contacted
  auto ids = request.server_ids();
  request.set_server_ids(ids.size(), 0);

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
