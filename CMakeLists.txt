cmake_minimum_required(VERSION 3.25.1)
project(Intel8080Emulator LANGUAGES C)
set(CMAKE_BUILD_TYPE Debug)
set(SOURCES
    src/main.c
)

add_executable(emulation ${SOURCES})