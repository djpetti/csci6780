add_subdirectory(proto)

add_executable(test_mp_client test_client.cpp)
target_link_libraries(test_mp_client gtest_main message_passing thread_pool
        loguru p4_test_proto wire_protocol)
add_test(NAME test_mp_client COMMAND test_mp_client)

add_executable(test_mp_server test_server.cpp)
target_link_libraries(test_mp_server gtest_main message_passing thread_pool
        loguru p4_test_proto wire_protocol)
add_test(NAME test_mp_server COMMAND test_mp_server)
