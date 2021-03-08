add_subdirectory(file_handler server_tasks)

add_library(server_lib server.cpp agent.cpp)
target_link_libraries(server_lib wire_protocol file_handler thread_pool)

add_executable(server server_main.cpp server_tasks/nport_task.cpp server_tasks/agent_task.cpp server_tasks/tport_task.cpp server_tasks/command_ids.cpp server_tasks/server_task.cpp)
target_link_libraries(server server_lib)