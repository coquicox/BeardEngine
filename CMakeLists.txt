cmake_minimum_required(VERSION 2.6)
project(BeardEngine)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(client)
add_subdirectory(server)
