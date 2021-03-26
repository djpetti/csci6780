include(FindProtobuf)
find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER test_messages.proto)
add_library(test_proto ${PROTO_HEADER} ${PROTO_SRC})
# Instruct everything that links to this to also link to Protobuf.
target_link_libraries(test_proto INTERFACE ${Protobuf_LIBRARIES})
# Make sure everything that depends on this can find the headers.
target_include_directories(test_proto INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

# For Mac Users
if (APPLE)
    target_include_directories(test_proto INTERFACE "/usr/local/Cellar/protobuf/3.14.0/include")
endif()
