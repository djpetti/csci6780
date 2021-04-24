/**
 * @file Message-passing utilities.
 */

#ifndef CSCI6780_MESSAGE_PASSING_UTILS_H
#define CSCI6780_MESSAGE_PASSING_UTILS_H

#include <netinet/in.h>

#include <cstdint>
#include <string>

namespace message_passing {

/**
 * @brief Creates an address structure to use for the socket.
 * @param port The port that we want to connect to.
 * @return The address structure that it created.
 */
struct sockaddr_in MakeAddress(uint16_t port);

/**
 * @brief Connects the socket to a remote server.
 * @param address The address structure specifying what to connect to.
 * @param hostname The hostname to connect to.
 * @return The FD of the client socket, or -1 on failure.
 */
int SetUpSocket(const sockaddr_in& address, const std::string& hostname);

/**
 * @brief Sets up a server socket for listening.
 * @param address The address structure to use.
 * @return The server socket it created, or -1 if it failed.
 */
int SetUpListenerSocket(const struct sockaddr_in& address);

}  // namespace message_passing

#endif  // CSCI6780_MESSAGE_PASSING_UTILS_H
