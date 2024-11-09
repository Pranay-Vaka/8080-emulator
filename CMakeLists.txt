cmake_minimum_required(VERSION 3.29)
project(8080 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(8080
    cpu.c
    disassembler.c
    invaders.h)
