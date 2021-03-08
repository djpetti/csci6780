add_library(server_tasks_lib nport_task.cpp tport_task.cpp
        command_ids.cpp server_task.cpp agent_task.cpp agent.cpp)

target_link_libraries(server_tasks_lib thread_pool wire_protocol file_handler)