#include <cstddef>
#include <string>
#include <thread>
#include <gtest/gtest.h>
#include <cstdint>
#include <vector>

#include "socket.hpp"

using http::socket_wrapper::Socket;


TEST(SocketTest, SocketEchoTest) {
    std::string host = "localhost";
    uint16_t port = 12345;

    std::string request = "marko";
    std::string response = "polo";

    std::thread server_thread([&]{
        Socket sock;
        sock.Bind(port, host);
        sock.Listen();
        std::string client_host;
        uint16_t client_port = 0;

        Socket client_socket = sock.Accept(client_host, client_port);

        std::vector<char> buf(request.size());

        size_t read_bytes = client_socket.Recv(
            buf.data(), request.size()
        );
        EXPECT_EQ(read_bytes, request.size());
        std::string r(buf.begin(), buf.end());
        EXPECT_EQ(request, r);
        size_t sent_bytes = client_socket.Send(response);
        EXPECT_EQ(sent_bytes, response.size());

    });

    std::thread client_thread([&]{
        Socket sock;
        sock.Connect(host, port);
        sock.Send(request);
        std::vector<char> buf(response.size());
        size_t read_bytes = sock.Recv(buf.data(), buf.size());
        EXPECT_EQ(read_bytes, response.size());
        std::string r(buf.begin(), buf.end());
        EXPECT_EQ(r, response);
    });

    server_thread.join();
    client_thread.join();
}
