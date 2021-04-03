include(FindProtobuf)
find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER ftp_messages.proto)
add_library(proto ${PROTO_HEADER} ${PROTO_SRC})
# Instruct everything that links to this to also link to Protobuf.
target_link_libraries(proto INTERFACE ${Protobuf_LIBRARIES})
# Make sure everything that depends on this can find the headers.
target_include_directories(proto INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

# For Mac Users
if (APPLE)
    file(GLOB protobuf_include_dirs "/usr/local/Cellar/protobuf/*/include")
    target_include_directories(proto INTERFACE ${protobuf_include_dirs})
endif()
