/**
 * @file Logic for handling individual connections to the server.
 */

#ifndef PROJECT1_AGENT_H
#define PROJECT1_AGENT_H

#include <cstdint>
#include <memory>
#include <vector>

#include "wire_protocol/wire_protocol.h"
#include "../file_handler/file_access_manager.h"
#include "../file_handler/file_handler_interface.h"
#include "../file_handler/thread_safe_file_handler.h"
#include "command_ids.h"
#include "ftp_messages.pb.h"

namespace server {

/**
 * @brief Contains logic for handling individual connections to the server.
 */
    class Agent {
    public:
        /**
         * @brief Constructor used for agents handling normal commands.
         * @param client_fd The FD of the client socket. Note that it will take
         *    responsibility for closing this socket on exit.
         * @param file_handler The `FileHandler` to use internally. Note this class
         *    will take ownership of it.
         * @param active_commands The list of active commands to be used internally.
         */
        Agent(int client_fd,
              std::unique_ptr<file_handler::ThreadSafeFileHandler> file_handler,
              std::shared_ptr<server_tasks::CommandIDs> active_commands);

        /**
         * @brief Constructor used for agents handling terminate commands.
         * @param client_fd The FD of the client socket. Note that it will take
         *    responsibility for closing this socket on exit.
         * @param active_commands The list of active commands to be used internally.
         */
        Agent(int client_fd,
              std::shared_ptr<server_tasks::CommandIDs> active_commands);


        ~Agent();

        /**
         * @brief Handles this particular client, until the client disconnects.
         * @note This will block and is meant to be run in its own thread..
         * @return True if the client was serviced and disconnected normally, false
         *    if a failure forced a premature disconnect.
         */
        bool Handle();



    private:

        /// Size in bytes to use for the internal message buffer.
        static constexpr size_t kClientBufferSize = 1000;

        /// Enumerates state of connected client.
        enum class ClientState {
            /// We expect more messages from the client.
            ACTIVE,
            /// The client has disconnected normally.
            DISCONNECTED,
            /// There was some error communicating with the client.
            ERROR,
        };

        /**
         * @brief Reads the next message from the socket.
         * @param message The message to read into.
         * @return True if it succeeded in reading the message, false otherwise.
         */
        ClientState ReadNextMessage(ftp_messages::Request *message);

        /**
         * @brief Reads a file contents message from the socket.
         * @param file_contents The message to read into.
         * @return True if it succeeded in reading the message, false if otherwise.
         */
        ClientState ReadFileContents(std::vector<uint8_t> *file_contents, uint16_t command_id);

        /**
         * @brief Dispatches an incoming request to the proper handler.
         * @param message The request.
         * @return The updated state of the client.
         */
        ClientState DispatchMessage(const ftp_messages::Request &message);

        /**
         * @brief All these methods are handlers for specific types of requests.
         * @param request The request to handle.
         * @return The updated state of the client.
         */
        ClientState HandleRequest(const ftp_messages::GetRequest &request);

        ClientState HandleRequest(const ftp_messages::PutRequest &request);

        ClientState HandleRequest(const ftp_messages::DeleteRequest &request);

        ClientState HandleRequest(const ftp_messages::ListRequest &request);

        ClientState HandleRequest(const ftp_messages::ChangeDirRequest &request);

        ClientState HandleRequest(const ftp_messages::MakeDirRequest &request);

        ClientState HandleRequest(const ftp_messages::PwdRequest &request);

        ClientState HandleRequest(const ftp_messages::QuitRequest &request);

        ClientState HandleRequest(const ftp_messages::TerminateRequest &request);

        /**
         * @brief Sends a response message to the client.
         * @param response The message to send.
         * @return The updated state of the client.
         */
        bool SendResponse(const ftp_messages::Response &response);

        /**
         * @brief Sends the file contents for a get request
         * @param file_contents the file contents to be sent
         * @return True on Success, False if the command was terminated
         */
        bool SendFileContents(const std::vector<uint8_t> &file_contents, uint16_t command_id);

        /// The FD to talk to the client on.
        int client_fd_;

        ///Active Commands @Note: To be inherited from the AgentTask
        std::shared_ptr<server_tasks::CommandIDs> active_commands_;

        /// Internal buffer to use for incoming messages.
        std::vector<uint8_t> incoming_message_buffer_{};
        /// Internal buffer to use for outgoing messages.
        std::vector<uint8_t> outgoing_message_buffer_{};
        /// Parser to use for reading messages on the socket.
        ::wire_protocol::MessageParser<ftp_messages::Request> parser_;

        /// Parser to use for reading file contents on the socket.
        ::wire_protocol::MessageParser<ftp_messages::FileContents> fc_parser_;

        /// Used for performing thread-safe filesystem operations.
        std::unique_ptr<file_handler::ThreadSafeFileHandler> file_handler_;


    };

}  // namespace server

#endif  // PROJECT1_AGENT_H
