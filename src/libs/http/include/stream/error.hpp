#pragma once

#include <stdexcept>
#include <string>

namespace http::stream
{
    class StreamError: public std::runtime_error {
    public:
    // Конструктор принимает описание ошибки
        explicit StreamError(const std::string& msg)
            : std::runtime_error(msg) {}
    };
} // namespace http::stream
