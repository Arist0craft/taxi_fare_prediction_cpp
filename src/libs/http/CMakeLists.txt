# библиотека HTTP будет статичной, вшиваем в бинарник
add_library(http STATIC 
    src/stream/reader.cpp
    src/socket.cpp
)

add_library(utils INTERFACE)

# Заголовки публичны для использования в проекте
target_include_directories(http PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

# target_link_libraries(http PRIVATE uriparser)
