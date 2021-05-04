cmake_minimum_required(VERSION 3.16)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

add_executable(HttpServer HttpServer_unittest.cpp)
target_link_libraries(HttpServer pthread FakeMuduoBase FakeMuduoNet)

add_executable(EchoServer EchoServer_unittest.cpp)
target_link_libraries(EchoServer pthread FakeMuduoBase FakeMuduoNet)
