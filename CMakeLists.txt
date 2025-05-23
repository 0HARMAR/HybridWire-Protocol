cmake_minimum_required(VERSION 3.10)
project(HybridWire-Protocol)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add include directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add source files
set(SOURCES
    src/hybridwire_protocol.cpp
    src/example.cpp
)

# Create executable
add_executable(hybridwire_example ${SOURCES})

# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(hybridwire_example PRIVATE -Wall -Wextra -Wpedantic)
endif()
