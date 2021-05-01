cmake_minimum_required(VERSION 3.16)
project(FakeMuduo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Wall")

add_executable(FakeMuduo main.cpp base/BoundBlockingQueue.h
        base/noncoapyable.h base/copyable.h base/CountDownLatch.h
        base/Exception.h base/Exception.cpp base/FileUtil.h
        base/FileUtil.cpp base/Logging.h base/Logging.cpp
        base/TimeStamp.h base/TimeStamp.cpp base/LogStream.h
        base/ThreadPool.h base/ThreadPool.cpp base/LogFile.h
        base/LogFile.cpp
        net/EventLoop.cpp net/EventLoop.h
        net/Callback.h net/Endian.h net/SocketsOps.h net/SocketsOps.cpp
        net/InetAddress.cpp net/InetAddress.h net/Socket.cpp
        net/Socket.h net/Channel.cpp net/Channel.h net/Poller.h
        net/Poller.cpp net/poller/EpollPoller.h net/poller/EpollPoller.cpp
        net/poller/DefaultPoller.cpp net/TcpConnection.cpp net/TcpConnection.h
        net/Buffer.cpp net/Buffer.h net/Acceptor.cpp net/Acceptor.h
        net/TcpServer.cpp net/TcpServer.h net/EventLoopThread.cpp
        net/EventLoopThread.h net/EventLoopThreadPool.cpp net/EventLoopThreadPool.h
        http/HttpRequest.h http/HttpRequest.cpp http/HttpResponse.cpp http/HttpResponse.h
        http/HttpContext.cpp http/HttpContext.h http/HttpServer.cpp http/HttpServer.h http/HttpBase.h)