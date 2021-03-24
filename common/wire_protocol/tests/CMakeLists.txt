add_executable(test_wire_protocol test_wire_protocol.cpp)
target_link_libraries(test_wire_protocol gtest_main proto wire_protocol
        ${PROTOBUF_LIBRARY})
add_test(NAME test_wire_protocol COMMAND test_wire_protocol)