add_executable(
    bot
    main.cpp
)

target_link_libraries(bot PRIVATE http)

target_include_directories(bot PRIVATE "${PROJECT_BINARY_DIR}")
