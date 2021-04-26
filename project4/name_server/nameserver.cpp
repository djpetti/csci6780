#include "nameserver.h"
namespace nameserver {
Nameserver::Nameserver(std::shared_ptr<thread_pool::ThreadPool> pool, const std::filesystem::path config_file) {
  threadpool_ = pool;
}

bool Nameserver::Enter() { return false; }

void Nameserver::Exit() {}

void Nameserver::HandleRequest(
    const google::protobuf::Message& request) {}

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
