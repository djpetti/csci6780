add_subdirectory(tests)

add_library(file_handler file_handler.cpp thread_safe_file_handler.cpp
        file_access_manager.cpp file_lock_guard.cpp)
target_link_libraries(file_handler loguru)
