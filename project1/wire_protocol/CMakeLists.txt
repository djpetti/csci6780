add_subdirectory(proto)
add_subdirectory(tests)

add_library(wire_protocol wire_protocol.cpp)
# Make sure we can access the generated protobuf files.
target_link_libraries(wire_protocol PUBLIC proto)

