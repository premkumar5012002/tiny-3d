cmake_minimum_required(VERSION 3.16)

project(tiny3D)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -v -g -Wall -I/opt/homebrew/include/SDL2 -D_THREAD_SAFE -L/opt/homebrew/lib -D_THREAD_SAFE")

add_executable(${PROJECT_NAME} src/main.c)

target_link_libraries(${PROJECT_NAME} SDL2)

