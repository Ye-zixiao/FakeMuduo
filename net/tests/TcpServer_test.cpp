//
// Created by Ye-zixiao on 2021/8/8.
//

#include "libfm/fmutil/Log.h"
#include "libfm/fmnet/EventLoop.h"
#include "libfm/fmnet/TcpServer.h"
using namespace fm::net;

class EchoServer {
 public:
  EchoServer(EventLoop *loop, const InetAddress &addr,
             const std::string& name)
      : loop_(loop),
        server_(loop, addr, name) {
    server_.setConnectionCallback(
        [this](const auto &conn) {
          this->onConnection(conn);
        });
    server_.setMessageCallback(
        [this](const auto &conn, auto *buffer, auto now) {
          this->onMessage(conn, buffer, now);
        });
  }

  void setThreadNum(int threads) { server_.setThreadNum(threads); }
  void start() { server_.start(); }

 private:
  void onMessage(const TcpConnectionPtr &conn, Buffer *buffer,
                 fm::time::TimeStamp now) {
    std::string msg(buffer->retrieveAllAsString());
    conn->send(msg);
  }

  void onConnection(const TcpConnectionPtr &conn) {
    if (conn->isConnected())
      LOG_INFO << "a new client is connected";
    else
      LOG_INFO << "a client is leave";
  }

 private:
  EventLoop *loop_;
  TcpServer server_;
};

int main() {
  fm::log::setLogLevel(fm::log::LogLevel::kDEBUG);
  fm::log::initialize(fm::log::StdLoggerTag{});

  EventLoop loop;
  InetAddress local_address(12000);
  EchoServer echo_server(&loop, local_address, "EchoServer");
  echo_server.start();
  loop.loop();

  return 0;
}