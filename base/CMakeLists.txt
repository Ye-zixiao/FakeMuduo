cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")

aux_source_directory(. DIR_LIB_SRCS)

add_library(FakeMuduoBase ${DIR_LIB_SRCS})