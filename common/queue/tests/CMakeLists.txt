add_executable(test_queue test_queue.cpp)
target_link_libraries(test_queue gtest_main queue)
add_test(NAME test_queue COMMAND test_queue)