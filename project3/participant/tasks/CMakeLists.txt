add_library(participant_tasks_lib console_task.cpp multicast_receiver_task.cpp)
target_link_libraries(participant_tasks_lib pub_sub_proto participant_lib thread_pool wire_protocol)
