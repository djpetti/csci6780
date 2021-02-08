add_subdirectory(file_handler)

add_library(server_lib server.cpp agent.cpp)
target_link_libraries(server_lib wire_protocol file_handler)

add_executable(server server_main.cpp)
target_link_libraries(server server_lib)