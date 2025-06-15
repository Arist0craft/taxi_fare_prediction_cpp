cmake_minimum_required(VERSION 3.31)
project(TelegramBot VERSION 0.0.1)

message(STATUS "CMAKE_VERSION: ${CMAKE_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# FetchContent_Declare(
#     uriparser
#     GIT_REPOSITORY https://github.com/uriparser/uriparser
#     GIT_TAG uriparser-0.9.8
# )
# set(URIPARSER_BUILD_TESTS OFF CACHE BOOL "Disable tests")

# FetchContent_MakeAvailable(uriparser)

configure_file(cmake/config.hpp.in config.hpp)

add_subdirectory(src/libs/http)
add_subdirectory(src/app)
add_subdirectory(tests)
