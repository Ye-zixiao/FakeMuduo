//
// Created by Ye-zixiao on 2021/5/1.
//

#ifndef LIBFM_HTTP_HTTPSERVER_H_
#define LIBFM_HTTP_HTTPSERVER_H_

#include <functional>
#include "libfm/base/NonCopyable.h"
#include "libfm/base/ThreadPool.h"
#include "libfm/net/Callback.h"
#include "libfm/net/TcpServer.h"

namespace fm::net::http {

class HttpRequest;
class HttpResponse;

class HttpServer : private NonCopyable {
 public:
  using HttpCallback = std::function<void(const HttpRequest &,
                                          HttpResponse *)>;

  HttpServer(EventLoop *loop,
             const InetAddress &listenAddr,
             const std::string &name,
             TcpServer::Option option = TcpServer::kNoReusePort,
             size_t threadPoolMaxSize = 10240);

  EventLoop *getLoop() const { return server_.getLoop(); }

  void setHttpCallback(const HttpCallback &cb) { httpCallback_ = cb; }

  void setThreadNum(int numIOThreads, int numThreadPools = 6);

  void start();

 private:
  void onConnection(const TcpConnectionPtr &conn);

  void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, time::Timestamp receivedTime);

  void onRequestInThreadPool(const TcpConnectionPtr &conn, HttpRequest request);
  void onRequest(const TcpConnectionPtr &conn, HttpRequest reqest);

 private:
  TcpServer server_;
  ThreadPool threadPool_;
  HttpCallback httpCallback_;
};

} // namespace fm::net::http

#endif //LIBFM_HTTP_HTTPSERVER_H_