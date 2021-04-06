add_subdirectory(coordinator_resources)

add_library(coordinator_lib coordinator.cpp coordinator.cpp
        coordinator_task.cpp coordinator_main.cpp
        coordinator_driver.cpp)
target_link_libraries(coordinator_lib coordinator_resources pub_sub_proto wire_protocol loguru thread_pool)

add_executable(coordinator coordinator_main.cpp)
target_link_libraries(coordinator coordinator_lib loguru)