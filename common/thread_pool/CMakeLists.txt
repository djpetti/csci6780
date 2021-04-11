add_subdirectory(tests)

add_library(thread_pool task.cpp thread_pool.cpp)
target_link_libraries(thread_pool loguru queue)
