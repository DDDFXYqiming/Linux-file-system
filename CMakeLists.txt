cmake_minimum_required(VERSION 3.16)
project(simdisk LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8 /W4)
else()
    add_compile_options(-Wall -Wextra)
endif()

add_library(simdisk_core STATIC
    src/filesystem.cpp
    src/commands.cpp
    src/auth.cpp
    src/ipc.cpp
)
target_include_directories(simdisk_core PUBLIC src)

# 控制台 CLI（跨平台）
add_executable(simdisk src/main.cpp)
target_link_libraries(simdisk PRIVATE simdisk_core)

# 后台内核模式（通过共享内存接收 shell 指令）
add_executable(simdisk-server src/server_main.cpp)
target_link_libraries(simdisk-server PRIVATE simdisk_core)

# Shell 客户端（与 simdisk-server 通过共享内存通信）
add_executable(simdisk-shell src/shell_main.cpp)
target_link_libraries(simdisk-shell PRIVATE simdisk_core)
