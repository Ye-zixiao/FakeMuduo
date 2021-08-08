cmake_minimum_required(VERSION 3.16)

add_executable(TcpServer_test TcpServer_test.cpp)
target_link_libraries(TcpServer_test fmutil fmnet)

add_executable(TimerQueue_test TimerQueue_test.cpp)
target_link_libraries(TimerQueue_test fmutil fmnet)
