#include "bootstrap.h"

namespace nameserver {
Bootstrap::Bootstrap(std::shared_ptr<thread_pool::ThreadPool> pool, const std::filesystem::path config_file)
    : Nameserver(pool, config_file) {

}

void Bootstrap::HandleRequest(const google::protobuf::Message& request) {
  /// Handle as if BootstrapMessage, else call:
  /// nameserver::Nameserver::HandleRequest(request);
}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::EntranceRequest& request) {}

void Bootstrap::InitiateEntrance(const message_passing::Endpoint server) {}

std::string Bootstrap::LookUp(int key) {
  return "";
}

void Bootstrap::Insert(int key, std::string val) {}

void Bootstrap::Delete(int key, std::string val) {}

}  // namespace nameserver
