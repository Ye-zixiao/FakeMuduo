// Author: Ye-zixiao
// Date: 2020-04-07

#include "../../src/FakeMuduo.h"

#include <unistd.h>
#include <iostream>

using namespace std;
using namespace fm;
using namespace fm::net;

void messageCallback(const TcpConnectionPtr &conn, Buffer *buffer, TimeStamp now) {
  std::string message = buffer->retrieveAllAsString();
  LOG_INFO << "get data from connection {" << conn->peerAddr().toIpPortStr()
           << "}: " << message;
  conn->send(message);
}

void connectionCallback(const TcpConnectionPtr &conn) {
  if (conn->isConnected()) {
    LOG_INFO << "connection " << conn->name() << " is connected!";
  } else {
    LOG_INFO << "connection " << conn->name() << " is disconnected";
  }
}

int main() {
  Logger::setLogLevel(Logger::TRACE);
  InetAddress listenAddr(12000);
  EventLoop loop;
  TcpServer tcp_server(&loop, listenAddr, "TestServer");
  tcp_server.setMessageCallback(messageCallback);
  tcp_server.setConnectionCallback(connectionCallback);
  tcp_server.start();
  loop.loop();
  LOG_INFO << "main thread end";
  return 0;
}