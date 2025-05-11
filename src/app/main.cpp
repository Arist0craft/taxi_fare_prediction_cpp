#include <iostream>
#include <string>
#include <sstream>
#include <config.hpp>

#include <socket.hpp>
#include <vector>


const uint16_t PORT = 8080;
const std::string RESPONSE = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n"
    "<html><body><h1>Hello from C++ HTTP Server!</h1></body></html>";

void handle_client(http::socket_wrapper::Socket& client_socket) {
    // Чтение запроса (пропустим анализ для упрощения)
    std::vector<char> buffer(1024);
    client_socket.Recv(buffer, buffer.size());

    for (auto& c: buffer) {
        std::cout << c;
    }
    std::cout << std::endl;
    // Отправка ответа
    client_socket.Send(RESPONSE);
}

int main() {
    // Создание сокета
    std::cout << "Project version:" << APP_VERSION << std::endl;
    
    http::socket_wrapper::Socket server_socket;
    server_socket.Create();
    server_socket.Bind(PORT);
    server_socket.Listen();

    std::cout << "Server listening on port " << PORT << "..." << std::endl;

    // Основной цикл сервера
    while (true) {
        std::string client_host;
        uint16_t client_port = 0;
        http::socket_wrapper::Socket client_socket = server_socket.Accept(client_host, client_port);
        std::cout << client_host << ' ' << client_port << std::endl;
        handle_client(client_socket);
    }

    return 0;
}