cmake_minimum_required(VERSION 3.16)

project(tiny3D)

find_package(SDL2 REQUIRED)

add_executable(${PROJECT_NAME} src/main.c)