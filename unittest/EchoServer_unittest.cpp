// Author: Ye-zixiao
// Date: 2020-04-07

#include "base/ThreadPool.h"
#include "base/Logging.h"
#include "base/LogFile.h"
#include "base/TimeStamp.h"
#include "net/InetAddress.h"
#include "net/Socket.h"
#include "net/EventLoop.h"
#include "net/Channel.h"
#include "net/Buffer.h"
#include "net/TcpServer.h"

#include <functional>

using namespace std;
using namespace fm;
using namespace fm::net;

class EchoServer {
 public:
  EchoServer(EventLoop *loop, const InetAddress &addr, const string &str)
	  : loop_(loop), server_(loop, addr, str) {
	server_.setConnectionCallback(
		std::bind(&EchoServer::onConnection, this, _1));
	server_.setMessageCallback(
		std::bind(&EchoServer::onMessage, this, _1, _2, _3));
  }

  void setThreadsNum(int threads) { server_.setThreadNum(threads); }
  void start() { server_.start(); }

 private:
  void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, TimeStamp now) {
	string msg(buffer->retrieveAllAsString());
	conn->send(msg);
//    buffer->retrieveAll();
  }

  void onConnection(const TcpConnectionPtr &conn) {
	if (conn->isConnected()) {
	  LOG_INFO << "a new client is connected";
	} else {
	  LOG_INFO << "a client is leave";
//      loop_->quit();
	}
  }

 private:
  EventLoop *loop_;
  TcpServer server_;
};

int main() {
//  Logger::setLogLevel(Logger::TRACE);

  EventLoop loop;
  InetAddress localAddress(12000);
  EchoServer echo_server(&loop, localAddress, "EchoServer");
//  echo_server.setThreadsNum(2);
  echo_server.start();
  loop.loop();

  return 0;
}