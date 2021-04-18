include(FindProtobuf)
find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRC PROTO_HEADER consistent_hash_msgs.proto)
add_library(consistent_hash_msgs ${PROTO_HEADER} ${PROTO_SRC})
# Instruct everything that links to this to also link to Protobuf.
target_link_libraries(consistent_hash_msgs INTERFACE ${Protobuf_LIBRARIES})
# Make sure everything that depends on this can find the headers.
target_include_directories(consistent_hash_msgs INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Ignore warnings on some versions of GCC.
    target_compile_options(consistent_hash_msgs PRIVATE -Wno-array-bounds)
endif()

# For Mac Users
if (APPLE)
    file(GLOB protobuf_include_dirs "/usr/local/Cellar/protobuf/*/include")
    target_include_directories(consistent_hash_msgs INTERFACE ${protobuf_include_dirs})
endif()
