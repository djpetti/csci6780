add_subdirectory(input_parser)
add_library(client_lib input_parser/input_parser.h client.cpp client.h)
target_link_libraries(client_lib wire_protocol file_handler input_parser)
add_executable(client client_main.cpp)
target_link_libraries(client client_lib)


