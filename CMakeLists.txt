cmake_minimum_required(VERSION 3.16)

project(FakeMuduo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")

add_executable(FakeMuduo main.cpp FakeMuduo.h)

aux_source_directory(. DIR_SRCS)
add_subdirectory(base)
add_subdirectory(net)

target_link_libraries(FakeMuduo FakeMuduoBase FakeMuduoNet)