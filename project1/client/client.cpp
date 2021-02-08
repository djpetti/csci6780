#include "client.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>


namespace client {
    using client::input_parser::InputParser;

    /**
     * @brief helper function for making a socket address structure
     *
     * @note could contain some fundamental errors...
     */
    struct sockaddr_in MakeAddress(uint16_t port) {
        struct sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        return address;
    }

    /**
     * @brief connects to socket
     * @param address socket address structure
     * @param name FTP Server IP Address
     * @return socket fd on success, -1 on failure
     *
     * @note could contain some fundamental errors...
     */
    int SetUpSocket(const struct sockaddr_in &address, std::string &name) {
        int sock = 0;

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket creation error");
            return -1;
        }

        if (inet_pton(AF_INET, name.c_str(), (struct sockaddr *) &address.sin_addr) <= 0) {
            perror("Invalid Address or Address not supported");
            return -1;
        }

        if (connect(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
            perror("Connection Failed");
            return -1;
        }


        return sock;
    }

    bool Client::Connect(std::string name, uint16_t port) {
        client_fd = SetUpSocket(MakeAddress(port), name);
        connected = true;
        return connected;
    }

    bool Client::SendReq() {
        if (!connected) {
            perror("Cannot Send Request... no longer connected to server");
        }
        if (send(client_fd, outgoing_msg_buf_.data(), outgoing_msg_buf_.size(), 0) < 0) {
            perror("Failed to send request.");
            return false;
        }
        return true;
    }

    void Client::HandleResponse() {
        output = "";

        //not sure if this is bad or not.. CLion yelled at me for not initializing.
        ftp_messages::Response *msg = nullptr;

        parser_.GetMessage(msg);

        //determine the type of response
        if (msg->has_get()) {

            //I'm not 100% about this casting but I think it's correct.
            ftp_messages::GetResponse *gResponse = (ftp_messages::GetResponse *) msg;

            //no, I don't use the FileHandler class I made. will change eventually
            std::ofstream new_file;
            new_file.open(ip->GetFilename());
            new_file << gResponse->file_contents();
        } else if (msg->has_list()) {

            //I'm not 100% about this casting but I think it's correct.
            ftp_messages::ListResponse *lResponse = (ftp_messages::ListResponse *) msg;

            //append filenames to output string separated by whitespace
            for (int i = 0; i < lResponse->filenames_size(); i++) {
                output.append(lResponse->filenames(i));
                if (!(i == lResponse->filenames_size() - 1)) {
                    output.append(" ");
                }
            }
        } else if (msg->has_pwd()) {

            //I'm not 100% about this casting but I think it's correct.
            ftp_messages::PwdResponse *pwdResponse = (ftp_messages::PwdResponse *) msg;
            output = pwdResponse->dir_name();
        }

        //make sure outputting is necessary
        if (output.compare("")) {
            //does nothing currently
            Output();
        }

    }

    void Client::FtpShell() {

        do {
            //reset input string
            std::string input = "";

            //will have to pay attention to formatting here when implementing Output()
            std::cout << "\nmyftp> ";

            //get input
            std::cin >> input;

            //determine command
            ip = new InputParser(input);
            InputParser::ReqType req = ip->GetReqType();

            //create & serialize request message for determined command
            switch (req) {
                case InputParser::GETF:
                    g = ip->CreateGetReq();
                    SerializeMessage(g);
                    break;
                case InputParser::PUTF:
                    p = ip->CreatePutReq();
                    SerializeMessage(p);
                    break;
                case InputParser::DEL:
                    del = ip->CreateDelReq();
                    SerializeMessage(del);
                    break;
                case InputParser::LS:
                    ls = ip->CreateListReq();
                    SerializeMessage(ls);
                    break;
                case InputParser::CD:
                    cd = ip->CreateCDReq();
                    SerializeMessage(cd);
                    break;
                case InputParser::MKDIR:
                    mkdir = ip->CreateMkdirReq();
                    SerializeMessage(mkdir);
                    break;
                case InputParser::PWD:
                    pwd = ip->CreatePwdReq();
                    SerializeMessage(pwd);
                    break;
                case InputParser::QUIT:
                    q = ip->CreateQuitReq();
                    SerializeMessage(q);

                    //close socket connection
                    close(client_fd);
                    connected = false;

                    //should skip this iteration of the loop,
                    //avoiding the functions called below.
                    //basically this should exit the do-while
                    continue;
            }
            SendReq();
            WaitForResponse();
            HandleResponse();

        } while (connected);


    }
}