cmake_minimum_required(VERSION 3.16)

set(SOURCE_FILES
        BoundBlockingQueue.h
        CountDownLatch.h
        Exception.h Exception.cpp
        FileUtil.h FileUtil.cpp
        LogFile.h LogFile.cpp
        Logging.h Logging.cpp
        LogStream.h
        SystemClock.h
        ThreadPool.h ThreadPool.cpp
        Timestamp.h Timestamp.cpp
        Copyable.h NonCopyable.h Movable.h)

add_library(fmbase SHARED ${SOURCE_FILES})
target_link_libraries(fmbase pthread)