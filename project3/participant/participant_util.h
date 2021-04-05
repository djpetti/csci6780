#ifndef PROJECT3_PARTICIPANT_UTIL_H
#define PROJECT3_PARTICIPANT_UTIL_H
#include <cstdint>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>

#include <pub_sub_messages.pb.h>
#include "wire_protocol/wire_protocol.h"

namespace participant_util {

/**
 * @param port the port of the address
 * @return an Internet socket struct
 */
struct sockaddr_in MakeAddress(uint16_t port);

/**
 * @param address the address to connect to
 * @param hostname the host to connect to
 * @return the socket file descriptor
 */
int SetUpSocket(const struct sockaddr_in &address,
                       const std::string &hostname);

/**
 * @param address the address to listen to
 * @return the socket file descriptor
 */
int SetUpListenerSocket(const struct sockaddr_in &address);

/**
 * @brief Performs a recv call on the socket, ignoring timeouts.
 * @param socket The socket to receive on.
 * @param buffer The buffer to receive into.
 * @param length The maximum length to receive.
 * @param flags The flags to use.
 * @return The return value from the internal recv() call. -1 for Failure
 */
int ReceiveForever(int socket, void* buffer, size_t length, int flags);

/**
 * @brief Performs a send call on the socket, ignoring timeouts, until
 *  all data is sent or it encounters an error.
 * @param socket The socket to send on.
 * @param buffer The buffer to send from.
 * @param length The maximum length to send.
 * @param flags The flags to use.
 * @return The return value from the internal send() call. -1 for Failure
 */
int SendForever(int socket, const void* buffer, size_t length, int flags);

}  // namespace participant_util

#endif  // PROJECT3_PARTICIPANT_UTIL_H
