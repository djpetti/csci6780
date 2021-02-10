#ifndef PROJECT1_INPUT_PARSER_H
#define PROJECT1_INPUT_PARSER_H

#include "../../wire_protocol/wire_protocol.h"
#include "../../server/file_handler/my_file_handler.h"
#include "ftp_messages.pb.h"
#include <sstream>
#include <cstdint>
#include <string>
#include <vector>

namespace client::input_parser {

    /**
     * @brief Assists in mapping input commands for FTP client to corresponding
     * protobuf requests.
     *
     * @note There's definitely some redundancy here. Will likely change
     * this class to have templated CreateReq functions in project 2.
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
        ~InputParser();

        /**
         * @brief tells user the request type
         * @return the request type. user relies on this information to determine
         * which CreateReq function to call
         */
        ReqType GetReqType();

        /**
         * @brief getter for filename
         * @return the filename
         */
        std::string GetFilename();
        /**
         * creates a get request
         * @return the request
         */
        ftp_messages::GetRequest CreateGetReq();

        /**
         * creates a put request
         * @return the request
         */
        ftp_messages::PutRequest CreatePutReq();

        /**
         * creates a delete request
         * @return the request
         */
        ftp_messages::DeleteRequest CreateDelReq();

        /**
         * creates a list request
         * @return the request
         */
        ftp_messages::ListRequest CreateListReq();

        /**
         * creates a cd request
         * @return the request
         */
        ftp_messages::ChangeDirRequest CreateCDReq();

        /**
         * creates a mkdir request
         * @return the request
         */
        ftp_messages::MakeDirRequest CreateMkdirReq();

        /**
         * creates a pwd request
         * @return the request
         */
        ftp_messages::PwdRequest CreatePwdReq();

        /**
         * creates a quit request
         * @return the request
         */
        ftp_messages::QuitRequest CreateQuitReq();

    private:


        const std::string commands[8] = {"get", "put", "delete", "ls", "cd", "mkdir", "pwd", "quit"};\

        //information to be extracted from input
        ReqType req;
        std::string fn;
        std::string dn;
        std::vector<uint8_t> contents;
    };
};//namespace input_parser
#endif //PROJECT1_INPUT_PARSER_H
