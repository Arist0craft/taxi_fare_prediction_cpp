#include "stream/reader.hpp"
#include "stream/error.hpp"

#include <algorithm>
#include <array>
#include <string>

namespace http::stream
{
    StreamReader::StreamReader(socket_wrapper::Socket& sock)
        : sock_(sock), buffer_(MAX_READER_BUFFER_SIZE) {}

    std::string StreamReader::ReadLine() {
        std::array<char, 2> crlf = {'\r', '\n'};

        while (write_pos_ - read_pos_ < MAX_LINE_LENGTH) {
            auto search_start_it = buffer_.begin() + start_search_pos;
            auto search_end_it = buffer_.begin() + write_pos_;

            auto it = std::search(
                search_start_it, 
                search_end_it, 
                crlf.begin(),
                crlf.end()
            );
            if (it != search_end_it) {
                std::string res(
                    buffer_.begin() + read_pos_, it
                );
                read_pos_ = std::distance(buffer_.begin(), it) + 2; // +2 to skip \r\n
                return res;
            }

            start_search_pos = write_pos_;
            ReadBytesToBuffer(BUFFER_READ_SIZE);
        }

        throw StreamError(
            "Client sent invalid or incomplete line (\\r\\n not found)"
        );
        return std::string();
    }

    std::string StreamReader::ReadBytes(size_t n) {
        if (n < 1) {
            sock_.Close();

        }
        return std::string();
    }

    void StreamReader::ReadBytesToBuffer(size_t n) {
        size_t bytes_read = 0;
        while (n > 0) {
            if (write_pos_ + n  > MAX_READER_BUFFER_SIZE) {
                    throw StreamError("Buffer overflow");
            }

            if (buffer_.size() < write_pos_ + n) {
                buffer_.resize(write_pos_ + n, 0);
            }

            bytes_read = sock_.Recv(
                buffer_.data() + write_pos_, n
            );

            if (bytes_read == 0) {
                throw StreamError("Connection closed while reading from stream");
            }
            write_pos_ += bytes_read;

            n -= bytes_read;
        }
    }

} // namespace http::stream
