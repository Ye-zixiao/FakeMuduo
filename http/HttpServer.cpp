//
// Created by Ye-zixiao on 2021/5/1.
//

#include "HttpServer.h"

#include "../base/Logging.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpContext.h"

using namespace fm;
using namespace fm::net;
using namespace fm::net::http;

static void defaultHttpCallback(const HttpRequest &request, HttpResponse *response) {
  response->setStatusCode(HttpResponse::k404NotFound);
  response->setStatusMessage("Not Found");
  response->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const std::string &name,
                       TcpServer::Option option)
    : server_(loop, listenAddr, name, option),
      httpCallback_(defaultHttpCallback) {
  server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, _1));
  server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

void HttpServer::start() {
  LOG_INFO << "HttpServer[" << server_.name()
           << "] starts listening on " << server_.ipPortStr();
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
    onRequest(conn, context->request());
    context->reset();
  }
}

// 在http报文解析之后处理请求，并回调用户层的具体处理函数
void HttpServer::onRequest(const TcpConnectionPtr &conn, const HttpRequest &request) {
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