# библиотека HTTP будет статичной, вшиваем в бинарник
add_library(http STATIC 
    src/message.cpp
    src/server.cpp
    src/socket.cpp
    src/client.cpp
)

# Заголовки публичны для использования в проекте
target_include_directories(http PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
