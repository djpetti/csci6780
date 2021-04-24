#include "server_task.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <loguru.hpp>
#include <utility>
#include <vector>

#include "../utils.h"

namespace message_passing {
namespace {

/// Timeout to use when accepting connections.
const struct timeval kSelectTimeout = {1, 0};

/**
 * @brief Represents status of `accept()` calls.
 */
enum class AcceptStatus { SUCCESS, FAILURE, TIMEOUT };

/**
 * @brief Performs an `accept()` call with a timeout.
 * @param socket_fd The socket FD to accept on.
 * @param endpoint[out] Will be filled with the client information for the
 *  connection we accepted.
 * @param client_fd[out] Will be set to the FD of the connected client.
 * @return The resulting status.
 */
AcceptStatus AcceptWithTimeout(int socket_fd, Endpoint *endpoint,
                               int *client_fd) {
  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(socket_fd, &read_fds);

  // Wait for a connection with a timeout.
  auto timeout = kSelectTimeout;
  const int kNumReady =
      select(socket_fd + 1, &read_fds, nullptr, nullptr, &timeout);
  if (kNumReady < 0) {
    LOG_S(ERROR) << "select() failed: " << std::strerror(errno);
    return AcceptStatus::FAILURE;
  } else if (kNumReady == 0) {
    // Timed out.
    return AcceptStatus::TIMEOUT;
  }

  // Accept the connection.
  struct sockaddr_in client_address {};
  socklen_t address_size = sizeof(client_address);
  *client_fd =
      accept(socket_fd, reinterpret_cast<struct sockaddr *>(&client_address),
             &address_size);
  if (*client_fd < 0) {
    LOG_S(ERROR) << "accept() failed: " << std::strerror(errno);
    return AcceptStatus::FAILURE;
  }

  // Fill in the client information.
  endpoint->hostname = inet_ntoa(client_address.sin_addr);
  endpoint->port = client_address.sin_port;
  LOG_S(INFO) << "Accepting new connection from " << endpoint->hostname << ":"
              << endpoint->port << ".";

  return AcceptStatus::SUCCESS;
}

}  // namespace

ServerTask::ServerTask(
    uint16_t listen_port, std::shared_ptr<thread_pool::ThreadPool> thread_pool,
    std::shared_ptr<queue::Queue<ReceiverTask::ReceiveQueueMessage>>
        receive_queue,
    NewClientCallback new_client_callback,
    SenderTask::SendCallback send_callback)
    : listen_port_(listen_port),
      thread_pool_(std::move(thread_pool)),
      receive_queue_(std::move(receive_queue)),
      new_client_callback_(std::move(new_client_callback)),
      send_callback_(std::move(send_callback)) {}

thread_pool::Task::Status ServerTask::SetUp() {
  // Set up the server socket.
  const auto kAddress = MakeAddress(listen_port_);
  server_socket_ = SetUpListenerSocket(kAddress);
  if (server_socket_ < 0) {
    return Status::FAILED;
  }

  return Status::RUNNING;
}

thread_pool::Task::Status ServerTask::RunAtomic() {
  // Clean up any disconnected clients.
  CloseDisconnected();

  // Accept a new client.
  Endpoint client_endpoint;
  int client_fd;
  const auto kAcceptResult =
      AcceptWithTimeout(server_socket_, &client_endpoint, &client_fd);
  if (kAcceptResult == AcceptStatus::TIMEOUT) {
    // Timeout. We'll try again later.
    return Status::RUNNING;
  } else if (kAcceptResult == AcceptStatus::FAILURE) {
    return Status::FAILED;
  }

  // Create tasks to handle the client.
  auto send_queue =
      std::make_shared<queue::Queue<SenderTask::SendQueueMessage>>();
  auto sender_task =
      std::make_shared<SenderTask>(client_fd, send_queue, send_callback_);
  auto receiver_task = std::make_shared<ReceiverTask>(client_fd, receive_queue_,
                                                      client_endpoint);

  thread_pool_->AddTask(sender_task);
  thread_pool_->AddTask(receiver_task);
  tasks_.insert(sender_task);
  tasks_.insert(receiver_task);

  // Run the new client callback.
  new_client_callback_(client_endpoint, send_queue);

  return Status::RUNNING;
}

void ServerTask::CleanUp() {
  LOG_S(INFO) << "Server task is exiting, cancelling " << tasks_.size()
              << " tasks.";

  for (const auto &kTask : tasks_) {
    thread_pool_->CancelTask(kTask);
  }
  // Close the associated file descriptors. Note that we do this in a separate
  // loop to ensure that both the sending and receiving tasks are cancelled
  // before we close the file descriptors.
  for (const auto &kTask : tasks_) {
    close(kTask->GetFd());
  }

  // Finally, close the actual server socket.
  close(server_socket_);
}

int ServerTask::GetFd() const { return server_socket_; }

void ServerTask::CloseDisconnected() {
  std::vector<std::shared_ptr<ISocketTask>> deletable_tasks;

  for (const auto &kTask : tasks_) {
    if (thread_pool_->GetTaskStatus(kTask) != Status::RUNNING) {
      LOG_S(INFO) << "Client with FD " << kTask->GetFd()
                  << " has disconnected.";

      // Clean up the socket.
      close(kTask->GetFd());
      // Mark the task as deletable.
      deletable_tasks.push_back(kTask);
    }
  }

  // Delete all the tasks that we can.
  for (const auto& kTask : deletable_tasks) {
    tasks_.erase(kTask);
  }
}

}  // namespace message_passing
