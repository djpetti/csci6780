/**
 * @brief Entry point for the FTP client.
 */


#include "client.h"
#include "input_parser/input_parser.h"
#define PORT 8080
namespace client {


    int main(int argc, const char **argv) {
        Client c;

        //second program argument should be the server port #
        const auto kPort = strtol(argv[1], nullptr, 10);

        //first program argument should be the server IP address
        c.Connect(argv[0], kPort);

        //spawn the shell
        c.FtpShell();
        return 0;
    }



}