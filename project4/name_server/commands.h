/**
 * @file The commands
 */
#ifndef PROJECT4_COMMANDS_H
#define PROJECT4_COMMANDS_H

#include <map>
#include <string>

namespace nameserver {

enum NameserverCommand { ENTER, EXIT };

const std::map<std::string, NameserverCommand> nameserver_cmds = {
    {"enter", ENTER}, {"exit", EXIT}};

enum BootstrapCommand { LOOKUP, INSERT, DELETE };

const std::map<std::string, BootstrapCommand> bootstrap_cmds = {
    {"lookup", LOOKUP}, {"insert", INSERT}, {"delete", DELETE}
};

}  // namespace nameserver

#endif  // PROJECT4_COMMANDS_H
