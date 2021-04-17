add_subdirectory(proto)

add_executable(test_message_sender test_message_sender.cpp)
target_link_libraries(test_message_sender gtest_main message_passing thread_pool
        loguru p4_test_proto wire_protocol)
add_test(NAME test_message_sender COMMAND test_message_sender)