add_library(coordinator_resources connected_participants.cpp
        message_log.cpp registrar.cpp
        messenger.cpp messenger_manager.cpp)
target_link_libraries(coordinator_resources pub_sub_proto wire_protocol loguru thread_pool)
