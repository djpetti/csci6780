add_library(message_passing_tasks sender_task.cpp receiver_task.cpp
        server_task.cpp)
target_link_libraries(message_passing_tasks thread_pool queue loguru)