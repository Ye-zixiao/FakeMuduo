cmake_minimum_required(VERSION 3.16)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

set(FM_LIBS fmbase fmnet)
set(FM_EXTRA_LIBS fmhttp)

# 添加HTTP服务器的目标
add_executable(WebServer WebServer.cpp)
target_link_libraries(WebServer pthread ${FM_LIBS} ${FM_EXTRA_LIBS})

add_executable(EchoServer EchoServer.cpp)
target_link_libraries(EchoServer pthread ${FM_LIBS})

add_executable(DatetimeServer DatetimeServer.cpp)
target_link_libraries(DatetimeServer pthread ${FM_LIBS})