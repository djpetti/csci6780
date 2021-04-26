/**
 * @file The commands
 */
#ifndef PROJECT4_COMMANDS_H
#define PROJECT4_COMMANDS_H

#include <unordered_map>
#include <string>

namespace nameserver {

enum class NameserverCommand { ENTER, EXIT };

const std::unordered_map<std::string, NameserverCommand> nameserver_cmds = {
    {"enter", NameserverCommand::ENTER},
    {"exit", NameserverCommand::EXIT}
};

enum class BootstrapCommand { LOOKUP, INSERT, DELETE };

const std::unordered_map<std::string, BootstrapCommand> bootstrap_cmds = {
    {"lookup", BootstrapCommand::LOOKUP},
    {"insert", BootstrapCommand::INSERT},
    {"delete", BootstrapCommand::DELETE}};

}  // namespace nameserver

#endif  // PROJECT4_COMMANDS_H
