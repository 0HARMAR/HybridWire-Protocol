cmake_minimum_required(VERSION 3.28)
project(HybridWireProtocol)

set(CMAKE_CXX_STANDARD 20)

add_executable(HybridWireProtocol
        include/hybridwire.hpp
        src/server.cpp
        src/hybridwire.cpp)
