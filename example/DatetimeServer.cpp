//
// Created by Ye-zixiao on 2021/6/7.
//

#include "libfm/base.h"
#include "libfm/net.h"

using namespace fm;
using namespace fm::net;

class DatetimeServer {
 public:
  DatetimeServer(EventLoop *loop,
                 const InetAddress &address,
                 const std::string &str)
      : server_(loop, address, str) {
    server_.setConnectionCallback([this](const TcpConnectionPtr &conn) {
      this->onConnection(conn);
    });
    server_.setMessageCallback(std::bind(&DatetimeServer::onMessage,
                                         this, _1, _2, _3));
  }

  void setThreadsNum(int threads) { server_.setThreadNum(threads); }
  void start() { server_.start(); }

 private:
  void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, time::Timestamp now) {
    // unlikely!
    buffer->retrieveAll();
  }

  void onConnection(const TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
      conn->send(time::SystemClock::now().toString());
    }
  }

 private:
  TcpServer server_;
};

int main() {
  EventLoop loop;
  InetAddress address(12000);
  DatetimeServer server(&loop, address, "DatetimeServer");
  server.start();
  loop.loop();

  return 0;
}