#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace http
{

using HttpHeaders = std::unordered_map<std::string, std::string>;

enum class HttpVersion {
    v_9 = 9,
    v_10 = 10,
    v_11 = 11,
    v_12 = 12,
    v_20 = 20
};

/**
 * @brief Base class of HttpMessage
 */
class HttpMessage {
public:

    HttpVersion GetVersion() const noexcept;
    void SetHeader(std::string key);
    std::string GetContent();

private:
    HttpHeaders headers_;
    std::string content_;
    HttpVersion version_;
};


/**
 * @brief HttpRequest
 */
class HttpRequest : HttpMessage {
public:
    HttpRequest();

private:
    std::string path_;
};

/**
 * @brief HttpResponse
 */
class HttpResponse : HttpMessage {
public:
    HttpResponse();

private:
    std::uint16_t status_code;
};
} // namespace http
