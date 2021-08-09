//
// Created by Ye-zixiao on 2021/8/8.
//

#include <string>
#include "libfm/fmnet/EventLoop.h"
#include "libfm/fmnet/TcpServer.h"
#include "libfm/fmutil/Log.h"

using namespace std;

class EchoServer {
 public:
  EchoServer(fm::net::EventLoop *loop,
             const fm::net::InetAddress &address,
             std::string name)
      : server_(loop, address, std::move(name)) {
    server_.setMessageCallback(onMessage);
    server_.setThreadNum(0);
  }

  void start() { server_.start(); }

 private:
  static void onMessage(const fm::net::TcpConnectionPtr &conn,
                        fm::net::Buffer *buffer, fm::time::TimeStamp) {
    conn->send(buffer);
  }

 private:
  fm::net::TcpServer server_;
};

int main() {
  fm::log::setLogLevel(fm::log::LogLevel::kDEBUG);
  fm::log::initialize(fm::log::StdLoggerTag{});

  fm::net::EventLoop loop;
  fm::net::InetAddress address(12000);
  EchoServer server(&loop, address, "EchoServer");
  server.start();
  loop.loop();

  return 0;
}