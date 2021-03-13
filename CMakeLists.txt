cmake_minimum_required(VERSION 2.8.9)

project(WebServer)

set(CXX_FLAGS -Wall -std=c++11 -D_PTHREADS -Wno-unused-parameter)

set(CXX_FLAGS
    -g
    -Wall
    -std=c++11
    -D_PTHREADS
    -Wno-unused-parameter
)


set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_subdirectory(base)
add_subdirectory(net)

add_executable(WebServer_cxz Main.cpp)

target_link_libraries(WebServer_cxz libserver_base libserver_net)

