# Download and unpack loguru at configure time
configure_file(loguru-download.cmake.in loguru-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/loguru-download )
if(result)
    message(FATAL_ERROR "CMake step for loguru failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/loguru-download )
if(result)
    message(FATAL_ERROR "Build step for loguru failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add loguru directly to our build.
add_library(loguru ${CMAKE_CURRENT_BINARY_DIR}/loguru-src/loguru.cpp)
# Make sure all dependencies can find the header.
target_include_directories(loguru INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/loguru-src)
# Enable logging with streams by default.
target_compile_definitions(loguru PUBLIC LOGURU_WITH_STREAMS=1)

# Link with external dependencies.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(loguru Threads::Threads ${CMAKE_DL_LIBS})
