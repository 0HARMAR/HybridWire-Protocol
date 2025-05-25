cmake_minimum_required(VERSION 3.10)
project(HybridWire-Protocol)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system)

# Add include directory
include_directories(${PROJECT_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})

# Add source files for the library
set(LIB_SOURCES
    src/server.cpp
    src/HybridClient.cpp
    src/protocol.cpp
)

# Create library
add_library(hwp STATIC ${LIB_SOURCES})
target_link_libraries(hwp PRIVATE ${Boost_LIBRARIES})

# Create examples directory if it doesn't exist
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples)

# Create examples
add_executable(http_example examples/http_example.cpp)
target_link_libraries(http_example PRIVATE hwp ${Boost_LIBRARIES})

# Add compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(hwp PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(http_example PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Add threading support
find_package(Threads REQUIRED)
target_link_libraries(http_example PRIVATE Threads::Threads)
