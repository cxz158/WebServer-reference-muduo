cmake_minimum_required(VERSION 2.8.9)

project(WebServer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -Wall -std=c++11 -D_PTHREADS -Wno-unused-parameter")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -O0")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2 ")

add_subdirectory(base)
add_subdirectory(net)

add_executable(WebServer_cxz Main.cpp)

target_link_libraries(WebServer_cxz libserver_base libserver_net)

