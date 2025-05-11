cmake_minimum_required(VERSION 3.31)
project(TelegramBot VERSION 0.0.1)

message(STATUS "CMAKE_VERSION: ${CMAKE_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

configure_file(cmake/config.hpp.in config.hpp)

add_subdirectory(src/libs/http)
add_subdirectory(src/app)

