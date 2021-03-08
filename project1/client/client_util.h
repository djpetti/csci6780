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
inline struct sockaddr_in MakeAddress(uint16_t port) {
  struct sockaddr_in address {};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  return address;
}

// utility function to initialize a socket
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

// utility function to write a file to the local system
inline void SaveIncomingFile(
    wire_protocol::MessageParser<google::protobuf::Message> f_parser,
    const std::string& name) {
  ftp_messages::FileContents contents;
  f_parser.GetMessage(&contents);
  std::ofstream new_file;
  new_file.open(name);
  new_file << contents.contents();
}
}
#endif  // PROJECT1_UTIL_H
