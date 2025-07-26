#include <cstddef>
#include <string>
#include <thread>
#include <chrono>
#include <gtest/gtest.h>
#include <cstdint>
#include <vector>

#include "socket.hpp"

using http::socket_wrapper::Socket;


TEST(SocketTest, GetLocalAddressTest) {
    Socket sock;
    sock.Create();
    sock.Bind(0, "127.0.0.1"); // Порт 0 - система выберет свободный
    
    auto [ip, port] = sock.GetLocalAddress();
    
    EXPECT_EQ(ip, "127.0.0.1");
    EXPECT_GT(port, 0); // Порт должен быть больше 0
    EXPECT_LT(port, 65536); // Порт должен быть в допустимом диапазоне
}

TEST(SocketTest, SocketEchoTest) {
    std::string host = "localhost";
    uint16_t actual_port = 0;

    std::string request = "marko";
    std::string response = "polo";

    std::thread server_thread([&]{
        Socket sock;
        sock.Bind(0, host); // Порт 0 - система выберет свободный
        sock.Listen();
        
        // Получаем реальный порт
        auto [server_ip, server_port] = sock.GetLocalAddress();
        actual_port = server_port;
        
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
        // Ждем, пока сервер получит порт
        while (actual_port == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        Socket sock;
        sock.Connect(host, actual_port);
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
