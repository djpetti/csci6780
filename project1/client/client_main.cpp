/**
 * @brief Entry point for the FTP client.
 */


#include "client.h"
#include "input_parser/input_parser.h"

#define PORT 8080


int main(int argc, const char **argv) {

    if (argc != 3) {
        std::cout << "\nIncorrect # of inputs. Server IP address & Port # expected";
        return 1;
    }
    client::Client ftp_client;

    //second program argument should be the server port #
    const auto kPort = strtol(argv[2], nullptr, 10);

    //first program argument should be the server IP address
    ftp_client.Connect(argv[1], kPort);

    //spawn the shell
    ftp_client.FtpShell();
    return 0;
}



