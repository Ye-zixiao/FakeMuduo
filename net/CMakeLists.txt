cmake_minimum_required(VERSION 3.16)

set(SOURCE_FILES
        Acceptor.h Acceptor.cpp
        Buffer.cpp
        Channel.cpp
        Epoller.h Epoller.cpp
        EventLoop.cpp
        EventLoopThread.cpp
        EventLoopThreadPool.h EventLoopThreadPool.cpp
        InetAddress.cpp
        Socket.h Socket.cpp
        SocketsOps.h SocketsOps.cpp
        TcpConnection.cpp
        TcpServer.cpp
        Timer.h Timer.cpp
        TimerQueue.h TimerQueue.cpp
        Connector.h Connector.cpp
        TcpClient.cpp
        http/HttpRequest.cpp http/HttpResponse.cpp
        http/HttpContext.h http/HttpContext.cpp
        http/HttpServer.cpp
        )

add_library(fmnet SHARED ${SOURCE_FILES})
target_link_libraries(fmnet pthread fmutil)

install(TARGETS fmnet LIBRARY DESTINATION lib)

add_subdirectory(tests)