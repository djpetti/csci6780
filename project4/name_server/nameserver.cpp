#include "nameserver.h"
namespace nameserver {
Nameserver::Nameserver(const std::filesystem::path config_file) {
  server_threadpool_ = std::make_shared<thread_pool::ThreadPool>();
  client_threadpool_ = std::make_shared<thread_pool::ThreadPool>();
}

bool Nameserver::Enter() { return false; }

void Nameserver::Exit() {}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::NameServerMessage& request) {}

void Nameserver::HandleRequest(
    const consistent_hash_msgs::EntranceInformation& request) {}

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
