cmake_minimum_required(VERSION 3.15)
project(csci6780 CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

include(googletest.cmake)
include(loguru.cmake)

include_directories(common)

add_subdirectory(common)
add_subdirectory(project1_2)
add_subdirectory(project3)