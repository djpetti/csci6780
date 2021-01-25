add_subdirectory(proto)

add_library(wire_protocol wire_protocol.cpp)
# Make sure we can access the generated protobuf files.
target_include_directories(wire_protocol PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(wire_protocol proto ${PROTOBUF_LIBRARY})