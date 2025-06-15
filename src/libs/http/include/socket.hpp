#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <sys/socket.h>

namespace http::socket_wrapper
{
    class SocketError: public std::runtime_error {
    public:
    // Конструктор принимает описание ошибки
        explicit SocketError(const std::string& msg)
            : std::runtime_error(msg) {}
    };

    class Socket {
    public:
        Socket();
        explicit Socket(int fd, bool is_closed, bool is_bound);
        ~Socket();

        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;
        Socket(Socket&& other) noexcept;
        Socket& operator=(Socket&& other) noexcept;

        void Close();
        void Create(
            int domain = AF_INET, int type = SOCK_STREAM, int protocol = 0
        );
        void Connect(const std::string& host, uint16_t port);

        size_t Send(const std::string& message);
        size_t Recv(char* buf, size_t max_bytes);

        void Bind(uint16_t port, const std::string& host = "0.0.0.0");
        void Listen(uint16_t backlog = 4096);
        Socket Accept(std::string& client_host, uint16_t& client_port);

    private:
        void ThrowError(const std::string& message); 
        void ThrowAddInfoError(const std::string& message, int status); 

        int fd_ = -1;
        bool is_closed = true;
        bool is_bound = false;
    };
} // namespace http::socket
