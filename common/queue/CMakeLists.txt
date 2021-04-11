add_subdirectory(tests)

add_library(queue INTERFACE)
target_link_libraries(queue INTERFACE loguru)