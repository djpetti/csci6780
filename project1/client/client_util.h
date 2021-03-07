#ifndef PROJECT1_UTIL_H
#define PROJECT1_UTIL_H
#include <cstdint>
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
namespace client_util {
inline struct sockaddr_in MakeAddress(uint16_t port) {
  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  return address;
}
inline int SetUpSocket(const struct sockaddr_in &address,
                       const std::string &hostname) {
  int sock = 0;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket creation error");
    return -1;
  }

  if (inet_pton(AF_INET, hostname.c_str(),
                (struct sockaddr *)&address.sin_addr) <= 0) {
    perror("Invalid Address or Address not supported");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("Connection Failed");
    return -1;
  }

  return sock;
}
}
#endif  // PROJECT1_UTIL_H
