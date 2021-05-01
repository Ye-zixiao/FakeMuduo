//
// Created by Ye-zixiao on 2021/5/1.
//

#ifndef FAKEMUDUO_HTTP_HTTPSERVER_H_
#define FAKEMUDUO_HTTP_HTTPSERVER_H_

#include <functional>

#include "../base/noncoapyable.h"
#include "../net/Callback.h"
#include "../net/TcpServer.h"

namespace fm {

namespace net {

namespace http {

class HttpRequest;
class HttpResponse;

class HttpServer : private noncopyable {
 public:
  using HttpCallback = std::function<void(const HttpRequest &,
                                          HttpResponse *)>;

  HttpServer(EventLoop *loop,
             const InetAddress &listenAddr,
             const std::string &name,
             TcpServer::Option option = TcpServer::kNoReusePort);

  EventLoop *getLoop() const { return server_.getLoop(); }

  // 对于业务逻辑层而言，只需要向HttpServer注册一个当请求到来时
  // 的处理函数即可实现一个最简单的http服务器
  void setHttpCallback(const HttpCallback &cb) { httpCallback_ = cb; }

  void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

  void start();

 private:
  void onConnection(const TcpConnectionPtr &conn);
  void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, TimeStamp receivedTime);
  void onRequest(const TcpConnectionPtr &conn, const HttpRequest &request);

 private:
  TcpServer server_;
  HttpCallback httpCallback_;
};

} // namespace http

} // namespace net

} // namespace fm

#endif //FAKEMUDUO_HTTP_HTTPSERVER_H_
