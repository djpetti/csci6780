include(FindProtobuf)
find_package(Protobuf REQUIRED)

add_subdirectory(tasks)
add_subdirectory(tests)

add_library(message_passing message_sender.cpp)
target_link_libraries(message_passing thread_pool ${Protobuf_LIBRARIES}
        wire_protocol loguru queue message_passing_tasks)