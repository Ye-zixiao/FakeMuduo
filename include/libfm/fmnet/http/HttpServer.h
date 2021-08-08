//
// Created by Ye-zixiao on 2021/8/8.
//

#ifndef OLD_LIBFM_INCLUDE_LIBFM_FMNET_HTTP_HTTPSERVER_H_
#define OLD_LIBFM_INCLUDE_LIBFM_FMNET_HTTP_HTTPSERVER_H_

#include <functional>
#include "libfm/fmutil/ThreadPool.h"
#include "libfm/fmnet/Callback.h"
#include "libfm/fmnet/TcpServer.h"

namespace fm {

namespace time {
class TimeStamp;
} // namespace fm::time

namespace net::http {

class HttpRequest;
class HttpResponse;

class HttpServer {
 public:
  using HttpCallback = std::function<void(const HttpRequest &,
                                          HttpResponse *)>;

  HttpServer(EventLoop *loop,
             const InetAddress &listenAddr,
             const std::string &name,
             TcpServer::Option option = TcpServer::kNoReusePort,
             size_t threadPoolMaxSize = 10240);

  HttpServer(const HttpServer &) = delete;
  HttpServer &operator=(const HttpServer &) = delete;

  EventLoop *getLoop() const { return server_.getLoop(); }

  void setHttpCallback(const HttpCallback &cb) { httpCallback_ = cb; }

  void setThreadNum(int numIOThreads, int numThreadPools = 6);

  void start();

 private:
  void onConnection(const TcpConnectionPtr &conn);

  void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, time::TimeStamp receivedTime);

  void onRequestInThreadPool(const TcpConnectionPtr &conn, HttpRequest request);
  void onRequest(const TcpConnectionPtr &conn, HttpRequest reqest);

 private:
  TcpServer server_;
  util::ThreadPool threadPool_;
  HttpCallback httpCallback_;
};

} // namespace fm::net::http

} // namespace fm

#endif //OLD_LIBFM_INCLUDE_LIBFM_FMNET_HTTP_HTTPSERVER_H_
