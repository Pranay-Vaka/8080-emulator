cmake_minimum_required(VERSION 3.25.1)
project(Intel8080Emulator LANGUAGES C)
set(CMAKE_BUILD_TYPE Debug)
set(SOURCES
    src/main.c
)

add_executable(target ${SOURCES})

add_custom_command(
    TARGET target POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/extras/test_binaries/invaders
        $<TARGET_FILE_DIR:target>/invaders
)