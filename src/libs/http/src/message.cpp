#include "message.hpp"

namespace http
{

/**
 * @brief Turns HttpVersion constant to string
 * @param v HTTP protocol version
 * @return string in format "HTTP/{version}"
 */
std::string to_string(HttpVersion v) {
    std::string base = "HTTP/";
    switch (v)
    {
    case HttpVersion::v_9 :
        return base + "0.9";

    case HttpVersion::v_10:
        return base + "1.0";

    case HttpVersion::v_11:
        return base + "1.1";

    case HttpVersion::v_12:
        return base + "1.2";
    
    case HttpVersion::v_20:
        return base + "2.0";

    default:
        return std::string();
    }
}

} // namespace http
