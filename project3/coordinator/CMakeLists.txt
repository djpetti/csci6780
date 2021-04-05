add_library(coordinator connected_participants.cpp message_log.cpp registrar.cpp messenger.cpp messenger_manager.cpp coordinator.cpp coordinator.cpp coordinator_task.cpp coordinator_main.cpp
        coordinator_driver.cpp)
target_link_libraries(coordinator pub_sub_proto wire_protocol loguru thread_pool)

add_executable(coordinator_main coordinator_main.cpp)
target_link_libraries(coordinator_main coordinator loguru)