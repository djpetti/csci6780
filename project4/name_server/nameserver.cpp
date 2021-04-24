#include "nameserver.h"

nameserver::Nameserver::Nameserver(const std::string& config_file) {

}

void nameserver::Nameserver::HandleRequest(
    const consistent_hash_msgs::NameServerMessage& request) {

}

void nameserver::Nameserver::HandleRequest(
    const consistent_hash_msgs::EntranceInformation& request) {

}

void nameserver::Nameserver::HandleRequest(
    const consistent_hash_msgs::ExitInformation& request) {

}

void nameserver::Nameserver::HandleRequest(
    const consistent_hash_msgs::UpdatePredecessorRequest& request) {

}

void nameserver::Nameserver::HandleRequest(
    const consistent_hash_msgs::UpdateSuccessorRequest& request) {

}

void nameserver::Nameserver::HandleRequest(
    const consistent_hash_msgs::UpdatePredecessorResponse& request) {

}

void nameserver::Nameserver::HandleRequest(
    const consistent_hash_msgs::LookUpResult& request) {

}

void nameserver::Nameserver::HandleRequest(
    const consistent_hash_msgs::InsertResult& request) {

}

void nameserver::Nameserver::HandleRequest(
    const consistent_hash_msgs::DeleteResult& request) {

}
void nameserver::Nameserver::ForwardRequest(
    bool to_predecessor, const consistent_hash_msgs::NameServerMessage& request) {

}
