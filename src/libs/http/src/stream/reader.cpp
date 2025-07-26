#include "stream/reader.hpp"

#include <algorithm>
#include <cstddef>
#include <string>

namespace http::stream
{
    StreamReader::StreamReader(socket_wrapper::Socket& sock)
        : sock_(sock), buffer_(MAX_READER_BUFFER_SIZE) {}
    
    std::string StreamReader::ReadUntil(const std::string& delimiter) {
        return ReadUntil(delimiter.begin(), delimiter.end());
    }

    std::string StreamReader::ReadLine() {
        return ReadUntil("\r\n");
    }

    std::string StreamReader::ReadBytes(size_t n) {
        while (buffer_.GetReadSize() < n) {
            auto read_bytes = ReadBytesToBuffer(n - buffer_.GetReadSize());
            if (read_bytes == 0) {
                throw StreamError("Not enough data to read");
            }
        
        }
        std::string res(n, '\0');
        buffer_.Read(res.begin(), n);
        return res;
    }

    size_t StreamReader::ReadBytesToBuffer(size_t n) {
        if (buffer_.GetWriteSize() < n) {
            throw StreamError("Not enough space in buffer");
        }

        n = std::min(n, buffer_.GetLinearWriteSize());

        n = sock_.Recv(buffer_.GetWritePtr(), n);
        buffer_.CommitWrite(n);
        return n;
    }

    std::string StreamReader::Peek(size_t n) {
        while (buffer_.GetReadSize() < n) {
            auto read_bytes = ReadBytesToBuffer(n - buffer_.GetReadSize());
            if (read_bytes == 0) {
                break;
            }
        }

        size_t to_read = std::min(n, buffer_.GetReadSize());
        std::string result(to_read, '\0');

        auto it = buffer_.begin();
        for (size_t i = 0; i < to_read; ++i, ++it) {
            result[i] = *it;
        }
        
        return result;
    }


    std::string StreamReader::Skip(size_t n) {
        size_t total_skipped = 0;
        
        while (total_skipped < n) {
            // Сначала пропускаем то, что есть в буфере
            size_t available_in_buffer = buffer_.GetReadSize();
            size_t to_skip_from_buffer = std::min(n - total_skipped, available_in_buffer);
            
            if (to_skip_from_buffer > 0) {
                buffer_.Skip(to_skip_from_buffer);
                total_skipped += to_skip_from_buffer;
            }
            
            // Если нужно пропустить больше, читаем новые данные
            if (total_skipped < n) {
                auto read_bytes = ReadBytesToBuffer(BUFFER_READ_SIZE);
                if (read_bytes == 0) {
                    break; // Больше данных нет
                }
            }
        }
        
        return std::to_string(total_skipped); // Возвращаем количество пропущенных байт
    }
} // namespace http::stream
