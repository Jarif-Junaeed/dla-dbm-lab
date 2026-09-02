cmake_minimum_required(VERSION 3.16)

project(dla LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    pcg-c-basic
    GIT_REPOSITORY https://github.com/imneme/pcg-c-basic.git
    GIT_TAG        bc39cd76ac3d541e618606bcc6e1e5ba5e5e6aa3
)
FetchContent_MakeAvailable(pcg-c-basic)

add_library(pcg_basic STATIC ${pcg-c-basic_SOURCE_DIR}/pcg_basic.c)
target_include_directories(pcg_basic PUBLIC ${pcg-c-basic_SOURCE_DIR})

add_executable(main
    src/main.c
    src/griddelta.c
)
target_include_directories(main PRIVATE include)
target_link_libraries(main PRIVATE pcg_basic)

target_compile_options(main PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)
