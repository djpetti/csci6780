add_executable(test_message_log test_message_log.cpp)
target_link_libraries(test_message_log gtest_main coordinator_resources)
add_test(NAME test_message_log COMMAND test_message_log)