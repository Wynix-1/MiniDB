#pragma once

#include "string.hpp"
#include "exception.hpp"
#include "logger.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

namespace mini_db {

class TCPClient {
public:
    TCPClient(const String& host = "127.0.0.1", int port = 8080)
        : host_(host), port_(port), socket_fd_(-1), connected_(false) {}

    ~TCPClient() {
        disconnect();
    }

    void connect() {
        socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd_ < 0) {
            throw Exception("Failed to create socket");
        }

        struct sockaddr_in server_addr;
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port_);

        if (inet_pton(AF_INET, host_.c_str(), &server_addr.sin_addr) <= 0) {
            throw Exception("Invalid address: " + host_);
        }

        if (::connect(socket_fd_, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            throw Exception("Failed to connect to server");
        }

        connected_ = true;
        LOG_INFO("Connected to server at " + host_ + ":" + String(std::to_string(port_).c_str()));
    }

    void disconnect() {
        if (socket_fd_ >= 0) {
            close(socket_fd_);
            socket_fd_ = -1;
        }
        connected_ = false;
        LOG_INFO("Disconnected from server");
    }

    String send_request(const String& request) {
        if (!connected_) {
            throw Exception("Not connected to server");
        }

        ssize_t bytes_sent = write(socket_fd_, request.c_str(), request.size());
        if (bytes_sent < 0) {
            throw Exception("Failed to send request");
        }

        LOG_DEBUG("Sent: " + request);

        char buffer[4096];
        std::memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(socket_fd_, buffer, sizeof(buffer) - 1);

        if (bytes_read < 0) {
            throw Exception("Failed to receive response");
        }

        String response(buffer);
        LOG_DEBUG("Received: " + response);

        return response;
    }

    bool is_connected() const { return connected_; }

private:
    String host_;
    int port_;
    int socket_fd_;
    bool connected_;
};

} // namespace mini_db
