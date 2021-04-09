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
  EchoServer(EventLoop *loop, const InetAddress &listenAddr, std::string name)
	  : tcp_server_(loop, listenAddr, name) {
	tcp_server_.setConnectionCallback(std::bind(&EchoServer::onConnection, this, _1));
	tcp_server_.setMessageCallback(std::bind(&EchoServer::onMessage, this, _1, _2, _3));
  }

  void setThreadNum(int numThreads) {
	tcp_server_.setThreadNum(numThreads);
  }

  void start() {
	tcp_server_.start();
  }

  void onMessage(const TcpConnectionPtr &conn, Buffer *buffer, TimeStamp now) {
	std::string msg(buffer->retrieveAllAsString());
	LOG_INFO << "get " << msg.size() << " bytes data from client {"
			 << conn->peerAddr().toIpPortStr() << "}: " << msg;
	conn->send(msg);
  }

  void onConnection(const TcpConnectionPtr &conn) {
	if (conn->isConnected()) {
	  LOG_INFO << "client {" << conn->peerAddr().toIpPortStr()
			   << "} is connected!";
	} else {
	  LOG_INFO << "client {" << conn->peerAddr().toIpPortStr()
			   << "} is disconnected!";
	}
  }

 private:
  TcpServer tcp_server_;
};

int main() {
  Logger::setLogLevel(Logger::TRACE);
  InetAddress listenAddr(12000);
  EventLoop loop;

  EchoServer echo_server(&loop, listenAddr, "EchoServer");
  echo_server.setThreadNum(4);
  echo_server.start();
  loop.loop();
  
  return 0;
}