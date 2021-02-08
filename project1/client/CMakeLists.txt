add_subdirectory(input_parser)
add_library(client input_parser/input_parser.h client.cpp client.h client_main.cpp)
target_link_libraries(client wire_protocol)
