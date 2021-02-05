cmake_minimum_required(VERSION 3.17)
project(project1 CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

include(googletest.cmake)

add_subdirectory(server)
add_subdirectory(wire_protocol)

add_executable(project1 main.cpp)