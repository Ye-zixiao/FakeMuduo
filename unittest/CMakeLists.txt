cmake_minimum_required(VERSION 3.16)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

set(FM_LIBS fmbase fmnet)
set(FM_EXTRA_LIBS fmjson fmhttp)

add_executable(Timestamp_unittest Timestamp_unittest.cpp)
target_link_libraries(Timestamp_unittest ${FM_LIBS})

add_executable(TimerQueue_unittest TimerQueue_unittest.cpp)
target_link_libraries(TimerQueue_unittest ${FM_LIBS})