cmake_minimum_required(VERSION 3.16)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

set(FAKE_MUDUO_LIBS FakeMuduoBase FakeMuduoNet)
set(FAKE_MUDUO_EXTRA_LIBS FakeMuduoHttp)

# 添加HTTP服务器的目标
add_executable(HttpServer HttpServer/WebServer.cpp)
target_link_libraries(HttpServer pthread ${FAKE_MUDUO_LIBS} ${FAKE_MUDUO_EXTRA_LIBS})

add_executable(EchoServer EchoServer/EchoServer.cpp)
target_link_libraries(EchoServer pthread ${FAKE_MUDUO_LIBS})

#add_executable(EventLoop_unittest EventLoop_unittest.cpp)
#target_link_libraries(EventLoop_unittest pthread ${FAKE_MUDUO_LIBS})

add_executable(LogFile_unittest UnitTest/LogFile_unittest.cpp)
target_link_libraries(LogFile_unittest pthread ${FAKE_MUDUO_LIBS})

add_executable(Sockets_unitest UnitTest/Sockets_unitest.cpp)
target_link_libraries(Sockets_unitest pthread ${FAKE_MUDUO_LIBS})

add_executable(TcpServer_unittest UnitTest/TcpServer_unittest.cpp)
target_link_libraries(TcpServer_unittest pthread ${FAKE_MUDUO_LIBS})

add_executable(TimerQueue_unittest UnitTest/TimerQueue_unittest.cpp)
target_link_libraries(TimerQueue_unittest pthread ${FAKE_MUDUO_LIBS})