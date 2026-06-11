#pragma once

#include "string.hpp"
#include "exception.hpp"
#include "logger.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <functional>

namespace mini_db {

class TCPServer {
public:
    using RequestHandler = std::function<String(const String&)>;

    TCPServer(int port = 8080) : port_(port), server_fd_(-1), running_(false) {}

    ~TCPServer() {
        stop();
    }

    void start(RequestHandler handler) {
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) {
            throw Exception("Failed to create socket");
        }

        int opt = 1;
        setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in address;
        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);

        if (bind(server_fd_, (struct sockaddr*)&address, sizeof(address)) < 0) {
            throw Exception("Failed to bind socket");
        }

        if (listen(server_fd_, 5) < 0) {
            throw Exception("Failed to listen");
        }

        running_ = true;
        LOG_INFO("Server started on port " + String(std::to_string(port_).c_str()));

        while (running_) {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            int client_fd = accept(server_fd_, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                if (running_) {
                    LOG_ERROR("Failed to accept connection");
                }
                continue;
            }

            LOG_INFO("Client connected");

            std::thread([this, client_fd, handler]() {
                handle_client(client_fd, handler);
            }).detach();
        }
    }

    void stop() {
        running_ = false;
        if (server_fd_ >= 0) {
            close(server_fd_);
            server_fd_ = -1;
        }
        LOG_INFO("Server stopped");
    }

    bool is_running() const { return running_; }

private:
    void handle_client(int client_fd, RequestHandler handler) {
        char buffer[4096];

        while (running_) {
            std::memset(buffer, 0, sizeof(buffer));
            int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

            if (bytes_read <= 0) {
                LOG_INFO("Client disconnected");
                close(client_fd);
                return;
            }

            String request(buffer);
            LOG_DEBUG("Received: " + request);

            String response = handler(request);

            ssize_t bytes_sent = write(client_fd, response.c_str(), response.size());
            if (bytes_sent < 0) {
                LOG_ERROR("Failed to send response");
                close(client_fd);
                return;
            }

            LOG_DEBUG("Sent: " + response);
        }

        close(client_fd);
    }

    int port_;
    int server_fd_;
    bool running_;
};

} // namespace mini_db
