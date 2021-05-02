//
// Created by Ye-zixiao on 2021/5/1.
//

#include "HttpServer.h"

#include "../../base/Logging.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpContext.h"

using namespace fm;
using namespace fm::net;
using namespace fm::net::http;

namespace {

void defaultHttpCallback(const HttpRequest &request, HttpResponse *response) {
  response->setStatusCode(HttpResponse::k404NotFound);
  response->setStatusMessage("Not Found");
  response->setCloseConnection(true);
}

} // unnamed namespace

HttpServer::HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const std::string &name,
                       TcpServer::Option option,
                       size_t threadPoolMaxSize)
    : server_(loop, listenAddr, name, option),
      threadPool_("ThreadPool", threadPoolMaxSize),
      httpCallback_(defaultHttpCallback) {
  server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, _1));
  server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

void HttpServer::setThreadNum(int numIOThreads, int numThreadPools) {
  server_.setThreadNum(numIOThreads);
  threadPool_.setThreadNum(numThreadPools);
}

void HttpServer::start() {
  LOG_INFO << "HttpServer[" << server_.name()
           << "] starts listening on " << server_.ipPortStr();
  threadPool_.start();
  server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr &conn) {
  if (conn->isConnected()) {
    conn->setUserContext(HttpContext());
  }
}

void HttpServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, TimeStamp receivedTime) {
  auto *context = std::any_cast<HttpContext>(conn->getMutableUserContext());

  if (!context->parseRequest(buffer, receivedTime)) {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }
  if (context->isAllDone()) {
    onRequest(conn, std::move(context->request()));
    context->reset();
  }
}

/**
 * 在工作线程池中处理如下部分的工作，具体包括http请求处理和响应发送工作，至于http请求报文
 * 的解析工作放到了I/O线程中处理，这要是考虑到对Buffer的线程安全性。因为有可能服务器收到
 * 客户的数据，单很有可能这些数据是不完整的，导致多个线程同时处理这个输入Buffer中的数据！
 * */
void HttpServer::onRequestInThreadPool(const TcpConnectionPtr &conn, HttpRequest request) {
  const std::string &connection = request.getHeader("Connection");
  bool close = connection == "close" || request.getVersion() == kHttp10;
  HttpResponse response(close);
  httpCallback_(request, &response);

  Buffer buffer;
  response.appendToBuffer(&buffer);
  conn->send(&buffer);
  if (response.isCloseConnection())
    conn->shutdown();
}

// 在http报文解析之后处理请求，并回调用户层的具体处理函数
void HttpServer::onRequest(const TcpConnectionPtr &conn, HttpRequest request) {
  threadPool_.run(std::bind(&HttpServer::onRequestInThreadPool,
                            this, conn, std::move(request)));
}