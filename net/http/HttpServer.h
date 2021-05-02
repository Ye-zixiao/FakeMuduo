//
// Created by Ye-zixiao on 2021/5/1.
//

#ifndef FAKEMUDUO_HTTP_HTTPSERVER_H_
#define FAKEMUDUO_HTTP_HTTPSERVER_H_

#include <functional>

#include "../../base/noncoapyable.h"
#include "../../base/ThreadPool.h"
#include "../Callback.h"
#include "../TcpServer.h"

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
             TcpServer::Option option = TcpServer::kNoReusePort,
             size_t threadPoolMaxSize = 10240);

  EventLoop *getLoop() const { return server_.getLoop(); }

  // 对于业务逻辑层而言，只需要向HttpServer注册一个当请求到来时的处理函数即可实现一个最简单的http服务器
  void setHttpCallback(const HttpCallback &cb) { httpCallback_ = cb; }

  void setThreadNum(int numIOThreads, int numThreadPools = 6);

  void start();

 private:
  void onConnection(const TcpConnectionPtr &conn);

  void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, TimeStamp receivedTime);

  void onRequestInThreadPool(const TcpConnectionPtr &conn, HttpRequest request);
  void onRequest(const TcpConnectionPtr &conn, HttpRequest reqest);

 private:
  TcpServer server_;
  ThreadPool threadPool_;
  HttpCallback httpCallback_;
};

} // namespace http

} // namespace net

} // namespace fm

#endif //FAKEMUDUO_HTTP_HTTPSERVER_H_
