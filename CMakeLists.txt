cmake_minimum_required(VERSION 3.25.1)
project(Intel8080Emulator)
set(CMAKE_BUILD_TYPE Debug)
add_executable(cpu cpu.c)
# Copy the invaders file to build directory
configure_file(
    ${CMAKE_SOURCE_DIR}/invaders
    ${CMAKE_CURRENT_BINARY_DIR}/invaders
    COPYONLY
)
