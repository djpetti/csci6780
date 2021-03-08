add_subdirectory(file_handler)
add_subdirectory(server_tasks)

add_library(server_lib server.cpp)
target_link_libraries(server_lib thread_pool server_tasks_lib)

add_executable(server server_main.cpp)
target_link_libraries(server server_lib)