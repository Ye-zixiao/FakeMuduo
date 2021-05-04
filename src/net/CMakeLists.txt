cmake_minimum_required(VERSION 3.16)

aux_source_directory(. DIR_LIB_SRCS)
aux_source_directory(poller DIR_LIB_SRCS)
aux_source_directory(http DIR_LIB_SRCS)

add_library(FakeMuduoNet SHARED ${DIR_LIB_SRCS})
target_link_libraries(FakeMuduoNet pthread)