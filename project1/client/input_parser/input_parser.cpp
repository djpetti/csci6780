#include "input_parser.h"

namespace client::input_parser {
    using ftp_messages::Request;
    using ftp_messages::GetRequest;
    using ftp_messages::PutRequest;
    using ftp_messages::ChangeDirRequest;
    using ftp_messages::ListRequest;
    using ftp_messages::PwdRequest;
    using ftp_messages::DeleteRequest;
    using ftp_messages::QuitRequest;
    using ftp_messages::MakeDirRequest;

    InputParser::InputParser(std::string &cmd) {
        this->fn = "";
        this->dn = "";

        //iterate through each word in input
        //the validity of this code relies on correct input syntax
        std::istringstream iss(cmd);
        do {
            std::string word;
            server::file_handler::MyFileHandler fh;
            iss >> word;
            for (int i = 0; i < commands->length(); i++) {

                //determine which command, extract further information if necessary
                if (!word.compare(commands[i])) {
                    switch (i) {
                        case 0:
                            iss >> this->fn;
                            this->req = GETF;
                            break;
                        case 1:
                            iss >> this->fn;
                            this->req = PUTF;
                            this->contents = fh.Get(fn);
                            break;
                        case 2:
                            iss >> this->fn;
                            this->req = DEL;
                            break;
                        case 3:
                            this->req = LS;
                            break;
                        case 4:
                            iss >> dn;
                            this->req = CD;
                            break;
                        case 5:
                            iss >>dn;
                            this->req = MKDIR;
                            break;
                        case 6:
                            this->req = PWD;
                            break;
                        case 7:
                            this->req = QUIT;
                            break;
                    }
                }
            }
        } while (iss);
    }// InputParser
    InputParser::ReqType InputParser::GetReqType(){return this->req;}// GetReqType()

    std::string InputParser::GetFilename() {
        return fn;
    }
    ftp_messages::GetRequest InputParser::CreateGetReq() {
        ftp_messages::GetRequest request;
        request.set_filename(this->fn);
        return request;
    }

    ftp_messages::PutRequest InputParser::CreatePutReq() {
        ftp_messages::PutRequest request;
        request.set_filename(this->fn);
        std::string c(contents.begin(),contents.end());
        request.set_file_contents(c);
        return request;
    }
    ftp_messages::DeleteRequest InputParser::CreateDelReq() {
        ftp_messages::DeleteRequest request;
        request.set_filename(this->fn);
        return request;
    }
    ftp_messages::ListRequest InputParser::CreateListReq() {
        ftp_messages::ListRequest request;
        return request;
    }
    ftp_messages::ChangeDirRequest InputParser::CreateCDReq() {
        ftp_messages::ChangeDirRequest request;

        //compare returns 0 on equal strings, hence '!'
        if (!this->dn.compare("..")){
            request.set_go_up(true);
        }
        else {
            request.set_go_up(false);
        }
        request.set_dir_name(this->dn);
        return request;
    }
    ftp_messages::MakeDirRequest InputParser::CreateMkdirReq() {
        ftp_messages::MakeDirRequest request;
        request.set_dir_name(this->dn);
        return request;
    }
    ftp_messages::PwdRequest InputParser::CreatePwdReq() {
        ftp_messages::PwdRequest request;
        return request;
    }
    ftp_messages::QuitRequest InputParser::CreateQuitReq() {
        ftp_messages::QuitRequest request;
        return request;
    }
}// namespace client::input_parser







































