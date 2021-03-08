cmake_minimum_required(VERSION 2.8.9)

project(WebServer)


set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g -O0 -Wall -std=c++11 -D_PTHREADS -Wno-unused-parameter")

set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall -std=c++11 -D_PTHREADS -Wno-unused-parameter")

add_subdirectory(base)
add_subdirectory(net)

add_executable(WebServer Main.cpp)

target_link_libraries(WebServer libserver_base libserver_net)

