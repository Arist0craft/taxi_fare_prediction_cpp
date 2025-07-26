#pragma once

#include <cstddef>
#include <iterator>
#include <string>

#include "socket.hpp"
#include "stream/error.hpp"
#include "utils/ring_buffer.hpp"


namespace http::stream
{
    constexpr size_t BUFFER_READ_SIZE = 4096; // 4 kB
    constexpr size_t MAX_READER_BUFFER_SIZE = 8 * 1024 * 1024; // 8 MB
    constexpr size_t MAX_LINE_LENGTH = 8192; // 8 kB

    
    class StreamReader {
    public:
        explicit StreamReader(http::socket_wrapper::Socket& sock);

        template<typename It>
        std::string ReadUntil(It begin, It end) {
            auto search_start_it = buffer_.begin();
            while (true) {
                auto search_end_it = buffer_.end();

                auto it = std::search(
                    search_start_it, 
                    search_end_it, 
                    begin,
                    end
                );

                if (it != search_end_it) {
                    auto to_read = std::distance(buffer_.begin(), it);
                    std::string res(to_read, '\0');
                    buffer_.Read(res.begin(),to_read);
                    buffer_.Skip(2);
                    return res;
                }

                auto old_size = buffer_.GetReadSize();
                search_start_it = (old_size > 0) ? buffer_.begin() + (old_size - 1) : buffer_.begin();

                auto read_size = ReadBytesToBuffer(BUFFER_READ_SIZE);
                if (buffer_.GetReadSize() >= MAX_LINE_LENGTH) {
                    throw StreamError("Line is too long");
                }
                if (read_size == 0) {
                    break;
                }
            }

            throw StreamError(
                "Client sent invalid or incomplete sequence"
            );
        }

        std::string ReadUntil(const std::string& delimiter);
        std::string ReadLine();
        std::string ReadBytes(size_t n);

        std::string Peek(size_t n);
        std::string Skip(size_t n);

    private:
        http::socket_wrapper::Socket& sock_;
        http::utils::RingBuffer<char> buffer_;
        
        size_t ReadBytesToBuffer(size_t n);
    };
} // namespace http::stream
