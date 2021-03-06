cmake_minimum_required(VERSION 2.8.9)

project(WebServer)

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb -lpthread")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall -lpthread")

add_subdirectory(base)
add_subdirectory(net)

add_executable(WebServer Main.cpp)

target_link_libraries(WebServer libserver_base libserver_net)

