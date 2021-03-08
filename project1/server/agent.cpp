#include "agent.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cstdio>
#include "stdlib.h"
#include <iostream>

namespace server {

    using file_handler::ThreadSafeFileHandler;
    using ftp_messages::Request;
    using ftp_messages::Response;

    namespace {

        /// Generic empty response.
        const Response kEmptyResponse{};

    }  // namespace

    Agent::Agent(int client_fd, std::unique_ptr<ThreadSafeFileHandler> file_handler,
                 std::shared_ptr<server_tasks::CommandIDs> active_commands)
            : client_fd_(client_fd), active_commands_(active_commands), file_handler_(std::move(file_handler)) {}

    Agent::Agent(int client_fd,
                 std::shared_ptr<server_tasks::CommandIDs> active_commands) :
            client_fd_(client_fd), active_commands_(active_commands) {}

    Agent::~Agent() {
        // Close the socket.
        close(client_fd_);
    }

    bool Agent::Handle() {
        ClientState client_state = ClientState::ACTIVE;


        while (client_state == ClientState::ACTIVE) {
            // Get the next message from the socket.
            Request message;
            client_state = ReadNextMessage(&message);

            if (client_state == ClientState::ACTIVE) {
                // Handle the message.
                client_state = DispatchMessage(message);
            }
        }

        return client_state != ClientState::ERROR;
    }

    Agent::ClientState Agent::ReadFileContents(ftp_messages::FileContents *fc, uint16_t command_id) {

        while (!fc_parser_.HasCompleteMessage()) {
            incoming_message_buffer_.resize(kClientBufferSize);

            uint8_t buf[1000];

            // Read 1000 bytes from the socket
            const auto bytes_read =
                    recv(client_fd_, buf, 1000, 0);

            if (bytes_read < 0) {
                // Failed to read anything.
                perror("Failed to read from client socket");
                fc_parser_.ResetParser();
                return ClientState::ERROR;
            } else if (bytes_read == 0) {
                // Client has disconnected nicely.
                std::cout << "Detected client disconnect." << std::endl;
                return ClientState::DISCONNECTED;
            }

            bool terminated = !active_commands_->Contains(command_id);

            // only continue if this put request is an active command
            if (!terminated) {

                // place data into the incoming message buffer
                for (int i = 0; i < 1000; i++) {
                    incoming_message_buffer_.push_back(buf[i]);
                }

            } else {
                std::cout << "Command #" << command_id << " successfully terminated." << std::endl;
            }

            // The parser assumes that the entire vector contains valid data, so limit
            // the size.
            incoming_message_buffer_.resize(bytes_read);

            // Add the data to the parser.
            fc_parser_.AddNewData(incoming_message_buffer_);
        }
        // Get the parsed message.
        if (!fc_parser_.GetMessage(fc)) {
            std::cerr << "Failed to get the parsed message." << std::endl;
            return ClientState::ERROR;
        }
        return ClientState::ACTIVE;
    }

    Agent::ClientState Agent::ReadNextMessage(Request *message) {
        while (!parser_.HasCompleteMessage()) {
            incoming_message_buffer_.resize(kClientBufferSize);

            // Read some more data from the socket.
            const auto bytes_read =
                    recv(client_fd_, incoming_message_buffer_.data(), kClientBufferSize, 0);
            if (bytes_read < 0) {
                // Failed to read anything.
                perror("Failed to read from client socket");
                parser_.ResetParser();
                return ClientState::ERROR;
            } else if (bytes_read == 0) {
                // Client has disconnected nicely.
                std::cout << "Detected client disconnect." << std::endl;
                return ClientState::DISCONNECTED;
            }

            // The parser assumes that the entire vector contains valid data, so limit
            // the size.
            incoming_message_buffer_.resize(bytes_read);

            // Add the data to the parser.
            parser_.AddNewData(incoming_message_buffer_);
        }

        // Get the parsed message.
        if (!parser_.GetMessage(message)) {
            std::cerr << "Failed to get the parsed message." << std::endl;
            return ClientState::ERROR;
        }
        return ClientState::ACTIVE;
    }

    Agent::ClientState Agent::DispatchMessage(const Request &message) {
        if (message.has_get()) {
            return HandleRequest(message.get());
        } else if (message.has_put()) {
            return HandleRequest(message.put());
        } else if (message.has_delete_()) {
            return HandleRequest(message.delete_());
        } else if (message.has_list()) {
            return HandleRequest(message.list());
        } else if (message.has_change_dir()) {
            return HandleRequest(message.change_dir());
        } else if (message.has_make_dir()) {
            return HandleRequest(message.make_dir());
        } else if (message.has_pwd()) {
            return HandleRequest(message.pwd());
        } else if (message.has_quit()) {
            return HandleRequest(message.quit());
        } else if (message.has_terminate()) {
            return HandleRequest(message.terminate());
        }

        std::cerr << "No valid message was received." << std::endl;
        return ClientState::ERROR;
    }

    bool Agent::SendResponse(const Response &response) {
        // Serialize the message.
        if (!wire_protocol::Serialize(response, &outgoing_message_buffer_)) {
            std::cerr << "Failed to serialize message." << std::endl;
            return false;
        }

        // Send the message.
        if (send(client_fd_, outgoing_message_buffer_.data(),
                 outgoing_message_buffer_.size(), 0) < 0) {
            perror("Failed to send the message");
            return false;
        }
        return true;
    }

    bool Agent::SendFileContents(const ftp_messages::FileContents &file_contents, uint16_t command_id) {
        // Serialize the message.
        if (!wire_protocol::Serialize(file_contents, &outgoing_message_buffer_)) {
            std::cerr << "Failed to serialize message." << std::endl;
            return false;
        }

        // monitor for termination request for get and put commands
        uint16_t total_bytes_sent = 0;

        // make 1000 byte buffer
        int num_bytes_to_send = 1000;
        uint8_t buf[num_bytes_to_send];

        // counter used to track when buf is filled
        int byte_counter = 0;

        bool terminated = false;

        // continue until we've sent the entire message
        while (total_bytes_sent < outgoing_message_buffer_.size()) {


            // fill the buffer one byte at a time
            for (int n:outgoing_message_buffer_) {

                // check if 1000 byte buffer is filled
                if (byte_counter == 1000) {
                    terminated = !active_commands_->Contains(command_id);

                    // check if the process has been terminated
                    if (!terminated) {
                        // Send 1000 bytes of the message.
                        if (send(client_fd_, buf,
                                 num_bytes_to_send, 0) < 0) {
                            perror("Failed to send the file contents.");
                            return false;
                        }
                    } else {
                        return false;
                    }

                    // reset
                    byte_counter = 0;

                    // update
                    total_bytes_sent += 1000;
                }
                buf[byte_counter] = outgoing_message_buffer_.at(n);
                ++byte_counter;


            }
        }
        return true;
    }

    Agent::ClientState Agent::HandleRequest(
            const ftp_messages::GetRequest &request) {
        std::cout << "Performing a GET operation." << std::endl;

        Response r;

        // generate a random # for the command id
        uint32_t id;
        do {
            // ensure this id is not already in use
            id = rand() % 1000000 + 1;
        } while (active_commands_->Contains(id));

        // register this command as an active command
        active_commands_->Insert(id);

        // make response
        r.mutable_get()->set_command_id(id);

        // Send client the command id for possible termination
        SendResponse(r);

        // Get the file data.
        const auto file_data = file_handler_->Get(request.filename());

        // Send the response.
        Response response;

        ftp_messages::FileContents fc;

        fc.set_contents(file_data.data(), file_data.size());

        return SendFileContents(fc, id) ? ClientState::ACTIVE : ClientState::ERROR;
    }

    Agent::ClientState Agent::HandleRequest(
            const ftp_messages::PutRequest &request) {
        std::cout << "Performing a PUT operation." << std::endl;

        Response r;
        uint32_t id;
        do {
            id = rand() % 1000000 + 1;
        } while (active_commands_->Contains(id));

        r.mutable_put()->set_command_id(id);

        // send client command id for possible termination
        SendResponse(r);

        // file contents to Put
        ftp_messages::FileContents fc;

        //Retrieve the file contents
        ReadFileContents(&fc, id);

        // Write the file data.
        std::vector<uint8_t> file_contents(fc.contents().begin(),
                                           fc.contents().end());

        if (!file_handler_->Put(request.filename(), file_contents)) {
            std::cerr << "Failed to write the file." << std::endl;
            return ClientState::ERROR;
        }

        return SendResponse(kEmptyResponse) ? ClientState::ACTIVE
                                            : ClientState::ERROR;
    }

    Agent::ClientState Agent::HandleRequest(
            const ftp_messages::DeleteRequest &request) {
        std::cout << "Performing a DELETE request." << std::endl;

        // Delete the file.
        if (!file_handler_->Delete(request.filename())) {
            std::cerr << "Failed to delete the file." << std::endl;
            return ClientState::ERROR;
        }

        return SendResponse(kEmptyResponse) ? ClientState::ACTIVE
                                            : ClientState::ERROR;
    }

    Agent::ClientState Agent::HandleRequest(
            const ftp_messages::ListRequest &request) {
        std::cout << "Performing a LIST request." << std::endl;

        // List the directory contents.
        const auto &kDirectoryContents = file_handler_->List();

        // Send the response.
        Response response;
        response.mutable_list()->clear_filenames();
        for (const auto &kFilename : kDirectoryContents) {
            response.mutable_list()->add_filenames(kFilename);
        }
        return SendResponse(response) ? ClientState::ACTIVE : ClientState::ERROR;
    }

    Agent::ClientState Agent::HandleRequest(
            const ftp_messages::ChangeDirRequest &request) {
        std::cout << "Performing a CD request." << std::endl;

        if (request.go_up()) {
            // Go up a directory.
            if (!file_handler_->UpDir()) {
                std::cerr << "Failed to go up a directory." << std::endl;
                return ClientState::ERROR;
            }
        } else {
            // Go down a directory.
            if (!file_handler_->ChangeDir(request.dir_name())) {
                std::cerr << "Failed to go down a directory." << std::endl;
                return ClientState::ERROR;
            }
        }

        return SendResponse(kEmptyResponse) ? ClientState::ACTIVE
                                            : ClientState::ERROR;
    }

    Agent::ClientState Agent::HandleRequest(
            const ftp_messages::MakeDirRequest &request) {
        std::cout << "Performing a MKDIR request." << std::endl;

        // Create the directory.
        if (!file_handler_->MakeDir(request.dir_name())) {
            std::cerr << "Failed to create a directory." << std::endl;
            return ClientState::ERROR;
        }

        return SendResponse(kEmptyResponse) ? ClientState::ACTIVE
                                            : ClientState::ERROR;
    }

    Agent::ClientState Agent::HandleRequest(
            const ftp_messages::PwdRequest &request) {
        std::cout << "Performing a PWD request." << std::endl;

        // Get the directory.
        const auto &kCurrentDir = file_handler_->GetCurrentDir();

        // Send the response.
        Response response;
        response.mutable_pwd()->set_dir_name(kCurrentDir);
        return SendResponse(response) ? ClientState::ACTIVE : ClientState::ERROR;
    }

    Agent::ClientState Agent::HandleRequest(
            const ftp_messages::TerminateRequest &request) {
        std::cout<< "Terminating an active GET or PUT command." << std::endl;

        // Retrieve the command id.
        std::string cmd_str = request.command_id();

        // Case to uint16_t
        int cmd_int(std::stoi(cmd_str));
        uint16_t cmd_id(0);

        if (cmd_int <= static_cast<int>(UINT16_MAX) && cmd_int >=0) {
            cmd_id = static_cast<uint16_t>(cmd_int);
        }
        else {
            std::cout << "Casting Error. " << std::endl;
        }

        // remove command from active command list.
        active_commands_->Delete(cmd_id);

        return ClientState::ACTIVE;

    }

    Agent::ClientState Agent::HandleRequest(
            const ftp_messages::QuitRequest &request) {
        std::cout << "Got a QUIT request. Exiting." << std::endl;
        // Indicate that we are finished with this client.
        return ClientState::DISCONNECTED;
    }



}  // namespace server
