#ifndef PROJECT1_INPUT_PARSER_H
#define PROJECT1_INPUT_PARSER_H

#include <ftp_messages.pb.h>

#include <array>
#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "../../server/file_handler/file_handler.h"
#include "../../wire_protocol/wire_protocol.h"
#include "ftp_messages.pb.h"

namespace client::input_parser {

    /**
     * @brief Assists in mapping input commands for FTP client to corresponding
     * protobuf request.
     *
     */
    class InputParser {
    public:
        enum ReqType {GETF, PUTF, DEL, LS, CD, MKDIR, PWD, QUIT};

        /**
         * @brief Initializes relevant info about this input command
         * @param cmd the user input
         * @note assumes correct syntax
         */
        InputParser(std::string& cmd);


        /**
         * @brief getter for filename
         * @return the filename
         */
        std::string GetFilename();

        /**
         * creates the request to be sent
         * @return
         */
        ftp_messages::Request CreateReq();


    private:
/**
         * creates a get request
         * @return the request
         */
        ftp_messages::Request CreateGetReq();

        /**
         * creates a put request
         * @return the request
         */
        ftp_messages::Request CreatePutReq();

        /**
         * creates a delete request
         * @return the request
         */
        ftp_messages::Request CreateDelReq();

        /**
         * creates a list request
         * @return the request
         */
        ftp_messages::Request CreateListReq();

        /**
         * creates a cd request
         * @return the request
         */
        ftp_messages::Request CreateCDReq();

        /**
         * creates a mkdir request
         * @return the request
         */
        ftp_messages::Request CreateMkdirReq();

        /**
         * creates a pwd request
         * @return the request
         */
        ftp_messages::Request CreatePwdReq();

        /**
         * creates a quit request
         * @return the request
         */
        ftp_messages::Request CreateQuitReq();

        const std::array<std::string, 8> commands_ = {"get", "put", "delete", "ls", "cd", "mkdir", "pwd", "quit"};\

        //information to be extracted from input
        ReqType req_;
        std::string fn_;
        std::string dn_;
        std::vector<uint8_t> contents_;
    };
};//namespace input_parser
#endif //PROJECT1_INPUT_PARSER_H
