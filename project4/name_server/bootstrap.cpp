#include "bootstrap.h"

nameserver::Bootstrap::Bootstrap(const std::filesystem::path config_file)
    : Nameserver(config_file) {

}

void nameserver::Bootstrap::HandleRequest(
    const consistent_hash_msgs::BootstrapMessage& request) {

}

void nameserver::Bootstrap::HandleRequest(
    const consistent_hash_msgs::EntranceRequest& request) {

}
