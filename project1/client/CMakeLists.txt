add_subdirectory(input_parser)
add_subdirectory(client_tasks)

add_library(client_lib client.cpp)
target_link_libraries(client_lib wire_protocol file_handler input_parser thread_pool
                      client_tasks_lib)
add_executable(client client_main.cpp)
target_link_libraries(client client_lib)


