cmake_minimum_required(VERSION 3.17)
project(project1 CXX)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(server)

add_executable(project1 main.cpp)