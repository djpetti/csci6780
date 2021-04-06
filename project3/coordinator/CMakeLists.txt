add_subdirectory(coordinator_resources)

add_library(coordinator coordinator.cpp coordinator.cpp
        coordinator_task.cpp coordinator_main.cpp
        coordinator_driver.cpp)
target_link_libraries(coordinator coordinator_resources pub_sub_proto wire_protocol loguru thread_pool)

add_executable(coordinator_main coordinator_main.cpp)
target_link_libraries(coordinator_main coordinator loguru)