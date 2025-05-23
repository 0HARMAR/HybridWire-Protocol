cmake_minimum_required(VERSION 3.28)
project(HybridWireProtocol)

set(CMAKE_CXX_STANDARD 20)

# 查找Boost库
find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

# 服务器可执行文件
add_executable(HybridWireProtocol
        include/hybridwire.hpp
        src/server.cpp
        src/hybridwire.cpp)

# 链接Boost库
target_link_libraries(HybridWireProtocol ${Boost_LIBRARIES})
