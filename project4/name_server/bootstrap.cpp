#include "bootstrap.h"

namespace nameserver {
Bootstrap::Bootstrap(std::shared_ptr<thread_pool::ThreadPool> pool,
                     std::shared_ptr<nameserver::tasks::ConsoleTask> console_task,
                     const std::filesystem::path config_file)
    : Nameserver(pool, console_task, config_file) {}

void Bootstrap::HandleRequest(const google::protobuf::Message& request) {
  /// Handle as if BootstrapMessage, else call:
  /// nameserver::Nameserver::HandleRequest(request);
}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::EntranceRequest& request) {}

void Bootstrap::InitiateEntrance(const message_passing::Endpoint server) {}

std::string Bootstrap::LookUp(int key) { return ""; }

void Bootstrap::Insert(int key, std::string val) {}

void Bootstrap::Delete(int key, std::string val) {}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::EntranceInformation& request) {}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::ExitInformation& request) {}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::LookUpResult& request) {}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::InsertResult& request) {}

void Bootstrap::HandleRequest(
    const consistent_hash_msgs::DeleteResult& request) {}

}  // namespace nameserver
