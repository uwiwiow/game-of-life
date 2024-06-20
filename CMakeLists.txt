cmake_minimum_required(VERSION 3.28)
project(gameOfLife C)

set(CMAKE_C_STANDARD 23)

find_package(raylib REQUIRED)

add_executable(gameOfLife main.c)

target_link_libraries(gameOfLife raylib GL m pthread dl rt X11)
