
include(GoogleTest)

add_executable(tests
    test_ring_buffer.cpp
    test_socket.cpp
)

target_include_directories(tests 
    PRIVATE ${CMAKE_SOURCE_DIR}/src/libs/http/include

)
target_link_libraries(tests gtest gtest_main http)

gtest_discover_tests(tests)

enable_testing()
# add_test(NAME govno COMMAND tests)