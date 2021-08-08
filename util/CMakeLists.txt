cmake_minimum_required(VERSION 3.16)

set(SOURCE_FILES
        AsyncLogger.h AsyncLogger.cpp
        BaseLogger.h
        Log.cpp
        LogBufferQueue.h LogBufferQueue.cpp
        LogFile.h LogFile.cpp
        LogLine.cpp
        SpinLock.h
        StdLogger.h StdLogger.cpp
        ThreadPool.cpp
        TimeStamp.cpp
        )

add_library(fmutil SHARED ${SOURCE_FILES})
target_link_libraries(fmutil pthread)

install(TARGETS fmutil LIBRARY DESTINATION lib)

add_subdirectory(tests)