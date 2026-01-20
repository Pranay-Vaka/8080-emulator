cmake_minimum_required(VERSION 4.25)
project(Intel8080Emulator LANGUAGES C)
set(CMAKE_BUILD_TYPE Debug)
set(SOURCES
    src/main.c
    src/cpu.c
    src/memory.c
    src/io.c
    src/flags.c
    src/interrupts.c
    src/cpu_arithmetic.c
    src/cpu_control.c
    src/cpu_data.c
    src/debug.c
)

add_executable(intel8080 ${SOURCES})

target_include_directories(intel8082 PRIVATE ${CMAKE_SOURCE_DIR}/include)

add_custom_command(
    TARGET intel8080 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/extras/test_binaries/invaders
        $<TARGET_FILE_DIR:intel8080>/invaders
)
