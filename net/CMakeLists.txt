cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")

aux_source_directory(. DIR_LIB_SRCS)
aux_source_directory(poller DIR_LIB_SRCS)
aux_source_directory(http DIR_LIB_SRCS)

add_library(FakeMuduoNet ${DIR_LIB_SRCS})