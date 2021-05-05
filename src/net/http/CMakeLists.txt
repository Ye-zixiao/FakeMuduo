cmake_minimum_required(VERSION 3.16)

aux_source_directory(. DIR_LIB_SRCS)

add_library(FakeMuduoHttp SHARED ${DIR_LIB_SRCS})
target_link_libraries(FakeMuduoHttp pthread)