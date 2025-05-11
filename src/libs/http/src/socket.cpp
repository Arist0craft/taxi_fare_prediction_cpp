#include "socket.hpp"

#include <iostream>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring> 
#include <cerrno>


namespace http::socket_wrapper
{
    Socket::Socket() {
        fd_ = socket(AF_UNSPEC, SOCK_STREAM, 0);
    }

    Socket::Socket(int fd, bool is_closed, bool is_bound)
        : fd_(fd), is_closed(is_closed), is_bound(is_bound) {}

    Socket::Socket(Socket&& other) noexcept
        : fd_(other.fd_), is_closed(other.is_closed), is_bound(other.is_bound)
    {
        other.fd_ = -1;
        other.is_closed = true;
        other.is_bound = false;
    }

    Socket::~Socket() {
      Close();
    }

    Socket& Socket::operator=(Socket&& other) noexcept {
        if (this != &other) {
            if (fd_ != 1) {
                Close();
            }

            fd_ = other.fd_;
            is_closed = other.is_closed;
            is_bound = other.is_bound;

            other.fd_ = -1;
            other.is_closed = true;
            other.is_bound = false;
        }
        return *this;
    }

    void Socket::Close() {
        if(!is_closed) {
            close(fd_);
            fd_ = -1;
            is_closed = true;
            is_bound = false;
        }
    }

    void Socket::Create(int domain, int type, int protocol) {
        using namespace std::string_literals;

        fd_ = socket(domain, type, protocol);
        if (fd_ == -1) {
            ThrowError("Socket creation failed"s);
        }
        is_closed = false;
    }

    void Socket::Connect(const std::string& host, uint16_t port) {
        using namespace std::string_literals;

        addrinfo hints{}, *result_addrinfo;

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        int status = 0;

        status = getaddrinfo(
            host.c_str(), std::to_string(port).c_str(), &hints, &result_addrinfo
        );
        if(status != 0) {
            freeaddrinfo(result_addrinfo);
            ThrowAddInfoError("Error while getting address info"s, status);
        };

        if (is_closed) {
            try {
                Create(
                    result_addrinfo->ai_family, 
                    result_addrinfo->ai_socktype, 
                    result_addrinfo->ai_protocol
                );
            } catch(const SocketError& e) {
                freeaddrinfo(result_addrinfo);
                throw;
            }
        }

        status = connect(
            fd_, 
            result_addrinfo->ai_addr, 
            result_addrinfo->ai_addrlen
        );
        freeaddrinfo(result_addrinfo);

        if(status != 0) {
            ThrowError("Error while connecting to host"s);
        };
    }

    size_t Socket::Send(const char* buf, size_t len) {
        using namespace std::string_literals;

        int bytes = send(fd_, buf, len, 0);
        if (bytes == -1) {
            ThrowError("Error while sending message"s);
        }
        return bytes;
    }

    size_t Socket::Recv(char* buf, size_t len) {
        using namespace std::string_literals;

        int bytes = recv(fd_, buf, len, 0);
        if (bytes == -1) {
            ThrowError("Error while reading message"s);
        }
        return bytes;
    }

    void Socket::Bind(uint16_t port, const std::string& host) {
        using namespace std::string_literals;

        addrinfo hints{}, *result_addrinfo;

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        int status = 0;
        status = getaddrinfo(
            host.c_str(), std::to_string(port).c_str(), &hints, &result_addrinfo
        );

        if(status != 0) {
            freeaddrinfo(result_addrinfo);
            ThrowAddInfoError("Error while getting address info"s, status);

        };

        status = bind(
            fd_, result_addrinfo->ai_addr, result_addrinfo->ai_addrlen
        );
        freeaddrinfo(result_addrinfo);

        if(status != 0) {
            ThrowError("Failed to bind to "s + host  + ":"s 
                + std::to_string(port));
        };
        is_bound = true;
    }

    void Socket::Listen(uint16_t backlog) {
        using namespace std::string_literals;

        if (!is_bound) {
            std::string message = "Error while listen: socket is not bound";
            std::cerr << message << "\n";
            throw SocketError(message);
        }

        int status = listen(fd_, backlog);
        if (status < 0) {
            ThrowError("Failed to listen"s);
        }
    }

    Socket Socket::Accept(std::string& client_host, uint16_t& client_port) {
        using namespace std::string_literals;

        sockaddr_storage client_addr{};
        socklen_t len = sizeof(client_addr);


        int client_fd = accept(
            fd_, reinterpret_cast<sockaddr*>(&client_addr), &len
        );
        if (client_fd < 0) {
            ThrowError("Failed to accept connection"s);
        }

        char host[NI_MAXHOST], serv[NI_MAXSERV];

        int result = getnameinfo(
            reinterpret_cast<sockaddr*>(&client_addr), 
            len,
            host,
            sizeof(host),
            serv,
            sizeof(serv),
            NI_NUMERICHOST | NI_NUMERICSERV
        );

        if (result != 0) {
            client_host = "unknown"s;
            client_port = 0;
        } else {
            client_host = host;
            client_port = static_cast<uint16_t>(std::stoi(serv));
        }

        return Socket(client_fd, false, false);
    }

    void Socket::ThrowError(const std::string& message) {
        using namespace std::string_literals;

        std::string error_message = message + ": "s 
            + std::string(strerror(errno));

        std::cerr << error_message << "\n";
        throw SocketError(error_message);
    }

    void Socket::ThrowAddInfoError(const std::string& message, int status) {
        using namespace std::string_literals;

        std::string error_message = message + ": "s 
            + std::string(gai_strerror(status));

        std::cerr << error_message << "\n";
        throw SocketError(error_message);
    }

}  // namespace http::socket_wrapper
