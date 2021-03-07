add_subdirectory(input_parser client_tasks)
add_library(client_lib input_parser/input_parser.h client.cpp client.h client_tasks client_util.h)
target_link_libraries(client_lib wire_protocol file_handler input_parser thread_pool)
add_executable(client client_main.cpp client_util.h)
target_link_libraries(client client_lib)


