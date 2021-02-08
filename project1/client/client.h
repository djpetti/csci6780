/**
 * @file FTP Client Class
 */
#ifndef PROJECT1_CLIENT_H
#define PROJECT1_CLIENT_H

#include <cstdint>
#include "../wire_protocol/wire_protocol.h"
#include "../server/file_handler/my_file_handler.h"
#include "ftp_messages.pb.h"
#include "input_parser/input_parser.h"

namespace client {

    using ftp_messages::Request;
    using ftp_messages::GetRequest;
    using ftp_messages::PutRequest;
    using ftp_messages::ChangeDirRequest;
    using ftp_messages::ListRequest;
    using ftp_messages::PwdRequest;
    using ftp_messages::DeleteRequest;
    using ftp_messages::QuitRequest;
    using ftp_messages::MakeDirRequest;

    using wire_protocol::MessageParser;

    /**
     * @brief FTP Client. Contains logic for connecting to server, sending requests, outputting responses,
     * and shell continuity.
     *
     * @note might rename to FTPClient or a better name
     */
    class Client {
    public:

        /**
         * @brief The shell logic.
         */
        void FtpShell();

        /**
         * @brief Handles output formatting for responses
         */
        void Output();

        /**
         *
         * @param name the ip address of the FTP server
         * @param port the port the FTP server is binded to
         * @return true on success, false on failure
         */
        bool Connect(std::string name, uint16_t port);

        /**
         * @brief serializes the message and store results to be used by SendReq()
         * @tparam T , expected to be of type ftp::messages
         * @param req the request type
         * @return true on success, false on failure
         */
        template<class T>
        bool SerializeMessage(T &req) {
            MessageParser<T> parser;

            const bool kSerializedSuccess = wire_protocol::Serialize(req, &outgoing_msg_buf_);
            parser.AddNewData(outgoing_msg_buf_);

            if (!kSerializedSuccess && !parser.HasCompleteMessage()) {
                return false;
            }
            return true;
        }

        /**
         * @brief sends the serialized message via a socket
         * @return true on success, false on failure
         */
        bool SendReq();

        /**
         * @brief expects a response message from socket, stores message in parser
         *
         */
        void WaitForResponse() {
            while (!parser_.HasCompleteMessage()) {
                incoming_msg_buf_.resize(kBufferSize);

                const auto bytes_read =
                        recv(client_fd, incoming_msg_buf_.data(), kBufferSize, 0);
                if (bytes_read < 0) {
                    //reset parser?
                } else if (bytes_read == 0) {
                    connected = false;
                }

                incoming_msg_buf_.resize(bytes_read);

                parser_.AddNewData(incoming_msg_buf_);
            }

        }

        /**
         * @brief extracts relevant information to be displayed to user from response
         */
        void HandleResponse();

    private:

        //tracking connection status
        bool connected;

        //buffer that stores serialized data to be sent to and received from server
        std::vector<uint8_t> outgoing_msg_buf_{};
        std::vector<uint8_t> incoming_msg_buf_{};

        //buffer size for client.
        static constexpr size_t kBufferSize = 4096;

        //client socket fd
        int client_fd;

        //response info to be formatted and outputted
        std::string output;

        //parser for handling messages
        MessageParser<ftp_messages::Response> parser_;

        //parser for user input
        client::input_parser::InputParser* ip;

        GetRequest g;
        PutRequest p;
        ChangeDirRequest cd;
        ListRequest ls;
        PwdRequest pwd;
        DeleteRequest del;
        QuitRequest q;
        MakeDirRequest mkdir;
        Request r;

    };
}

#endif //PROJECT1_CLIENT_H
