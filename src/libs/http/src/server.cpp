// #include "server.hpp"

// #include <iostream>
// #include <exception>
// #include <sys/socket.h>
// #include <unistd.h>

// namespace http
// {

// HttpServer::HttpServer(std::string &host, int port) {
//     server_file_descriptor_ = socket(AF_INET, SOCK_STREAM, 0);
//     if (server_file_descriptor_ == -1) {
//         throw std::runtime_error("Socker creation error");
//     }
// }

// HttpServer::~HttpServer() {
//     close(server_file_descriptor_);
// }



// } // namespace http
