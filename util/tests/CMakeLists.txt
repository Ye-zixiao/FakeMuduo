cmake_minimum_required(VERSION 3.16)

add_executable(TimeStamp_test TimeStamp_test.cpp)
target_link_libraries(TimeStamp_test fmutil)

add_executable(LogLine_test LogLine_test.cpp)
target_link_libraries(LogLine_test fmutil)

add_executable(StdLogger_test StdLogger_test.cpp)
target_link_libraries(StdLogger_test fmutil)

add_executable(LogBufferQueue_test LogBufferQueue_test.cpp)
target_link_libraries(LogBufferQueue_test fmutil)

add_executable(LogLine_thread_test LogLine_thread_test.cpp)
target_link_libraries(LogLine_thread_test fmutil)

add_executable(AsyncLogger_test AsyncLogger_test.cpp)
target_link_libraries(AsyncLogger_test fmutil)

add_executable(LogFile_test LogFile_test.cpp)
target_link_libraries(LogFile_test fmutil)