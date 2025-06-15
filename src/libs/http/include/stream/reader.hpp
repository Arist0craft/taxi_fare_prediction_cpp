#pragma once

#include <string>

#include "socket.hpp"
#include "utils/ring_buffer.hpp"


namespace http::stream
{
    constexpr size_t BUFFER_READ_SIZE = 4096; // 4 kB
    constexpr size_t MAX_READER_BUFFER_SIZE = 8 * 1024 * 1024; // 8 MB
    constexpr size_t MAX_LINE_LENGTH = 8192; // 8 kB


    class StreamReader {
    public:
        explicit StreamReader(http::socket_wrapper::Socket& sock);
        std::string ReadLine();
        std::string ReadBytes(size_t n);

    private:
        http::socket_wrapper::Socket& sock_;
        http::utils::RingBuffer<char> buffer_;
        void ReadBytesToBuffer(size_t n);
    };
} // namespace http::stream
