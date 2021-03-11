#ifndef PROJECT1_UTIL_H
#define PROJECT1_UTIL_H
#include <cstdint>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>

#include "ftp_messages.pb.h"
#include "../wire_protocol/wire_protocol.h"

namespace client_util {

// utility function to make an address struct based on port
struct sockaddr_in MakeAddress(uint16_t port);

// utility function to initialize a socket
int SetUpSocket(const struct sockaddr_in &address,
                       const std::string &hostname);

// utility function to write a file to the local system
void SaveIncomingFile(const std::string &contents,
                             const std::string &name);

/**
 * @brief Performs a recv call on the socket, ignoring timeouts.
 * @param socket The socket to receive on.
 * @param buffer The buffer to receive into.
 * @param length The maximum length to receive.
 * @param flags The flags to use.
 * @return The return value from the internal recv() call.
 */
int ReceiveForever(int socket, void* buffer, size_t length, int flags);

/**
 * @brief Performs a send call on the socket, ignoring timeouts, until
 *  all data is sent or it encounters an error.
 * @param socket The socket to send on.
 * @param buffer The buffer to send from.
 * @param length The maximum length to send.
 * @param flags The flags to use.
 * @return The return value from the internal send() call.
 */
int SendForever(int socket, const void* buffer, size_t length, int flags);

}  // namespace client_util

#endif  // PROJECT1_UTIL_H
