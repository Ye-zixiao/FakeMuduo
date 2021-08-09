cmake_minimum_required(VERSION 3.16)

add_executable(HttpServer
        HttpServer/HttpServer.cpp
        HttpServer/ReadSmallFile.h HttpServer/ReadSmallFile.cpp)
target_link_libraries(HttpServer fmutil fmnet)

add_executable(EchoServer EchoServer.cpp)
target_link_libraries(EchoServer fmutil fmnet)

add_executable(EchoClient EchoClient.cpp)
target_link_libraries(EchoClient fmutil fmnet)