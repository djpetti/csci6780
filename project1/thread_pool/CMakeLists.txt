add_subdirectory(tests)

add_library(thread_pool thread_pool_interface.h task.cpp thread_pool.cpp)
target_link_libraries(thread_pool loguru)
