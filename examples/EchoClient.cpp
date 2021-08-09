//
// Created by Ye-zixiao on 2021/8/8.
//

#include <string>
#include <iostream>
#include "libgen.h"
#include "libfm/fmutil/Log.h"
#include "libfm/fmutil/TimeStamp.h"
#include "libfm/fmnet/TcpClient.h"
#include "libfm/fmnet/EventLoop.h"
#include "libfm/fmnet/InetAddress.h"
#include "libfm/fmnet/TimerId.h"

using namespace std;
using namespace fm;

class EchoClient {
 public:
  EchoClient(net::EventLoop *loop,
             const net::InetAddress &address,
             std::string name,
             std::string msg)
      : loop_(loop),
        client_(loop, address, name),
        msg_(std::move(msg)) {
    client_.setConnectionCallback(
        [this](const auto &cn) { this->onConnection(cn); });
    client_.setMessageCallback(
        [this](const auto &cn, auto *buffer, auto now) {
          this->onMessage(cn, buffer, now);
        });
  }

  void connect() { client_.connect(); }

 private:
  void onConnection(const net::TcpConnectionPtr &conn) {
    if (conn->isConnected()) {
      LOG_INFO << "connect to server is successes";
      conn->setTcpNoDelay(true);
      conn->send(msg_);
    } else {
      loop_->quit();
    }
  }

  void onMessage(const net::TcpConnectionPtr &conn, net::Buffer *buffer,
                 time::TimeStamp now) {
    auto msg = buffer->retrieveAllAsString();
    std::cout << "received: " << msg << std::endl;
    loop_->runAfter(500ms, [conn, msg] { conn->send(msg); });
  }

 private:
  net::EventLoop *loop_;
  net::TcpClient client_;
  std::string msg_;
};

int main(int argc, char *argv[]) {
  if (argc < 4) {
    cerr << "usage: " << basename(argv[0]) << " <ipaddr> <port> <msg>" << endl;
    exit(EXIT_FAILURE);
  }

  fm::log::initialize(log::StdLoggerTag{});

  net::EventLoop loop;
  net::InetAddress address(argv[1], atoi(argv[2]));
  EchoClient client(&loop, address, "EchoClient", argv[3]);
  client.connect();
  loop.loop();
}