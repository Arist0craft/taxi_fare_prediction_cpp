#include "stream/reader.hpp"
#include "stream/error.hpp"

#include <string>

namespace http::stream
{
    StreamReader::StreamReader(socket_wrapper::Socket& sock)
        : sock_(sock), buffer_(MAX_READER_BUFFER_SIZE) {}

    std::string StreamReader::ReadLine() {
        
        return std::string();
    }

    std::string StreamReader::ReadBytes(size_t n) {
        return std::string();
    }

    void StreamReader::ReadBytesToBuffer(size_t n) {
        
    }

} // namespace http::stream
