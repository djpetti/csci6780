add_subdirectory(input_parser)
add_library(client input_parser/input_parser.h client.cpp client.h)
target_link_libraries(client wire_protocol file_handler input_parser)
add_executable(client_exe client_main.cpp)
target_link_libraries(client_exe client)


