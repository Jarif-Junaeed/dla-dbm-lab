cmake_minimum_required(VERSION 3.16)

project(dla LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(main main.c)

target_compile_options(main PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
