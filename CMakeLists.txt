cmake_minimum_required(VERSION 3.28)
project(HybridWireProtocol)

set(CMAKE_CXX_STANDARD 20)

add_executable(HybridWireProtocol src/main.cpp
        include/hybirdwire.hpp
        src/server.cpp)
