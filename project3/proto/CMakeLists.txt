include(FindProtobuf)
find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER pub_sub_messages.proto)
add_library(pub_sub_proto ${PROTO_HEADER} ${PROTO_SRC})
# Instruct everything that links to this to also link to Protobuf.
target_link_libraries(pub_sub_proto INTERFACE ${Protobuf_LIBRARIES})
# Make sure everything that depends on this can find the headers.
target_include_directories(pub_sub_proto INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

# For Mac Users
if (APPLE)
    file(GLOB protobuf_include_dirs "/usr/local/Cellar/protobuf/*/include")
    target_include_directories(pub_sub_proto INTERFACE ${protobuf_include_dirs})
endif()
