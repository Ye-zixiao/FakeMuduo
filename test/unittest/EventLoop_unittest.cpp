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

#include <unistd.h>

#include <iostream>

using namespace std;
using namespace fm;
using namespace fm::net;

EventLoop loop;
Socket listenSocket(sockets::createNonblockingSocketOrDie(AF_INET));

void handleRead(TimeStamp now) {
  InetAddress peerAddr;
  int connfd = listenSocket.accept(&peerAddr);
  LOG_INFO << "accept a client: " << peerAddr.toIpPortStr();
  sockets::write(connfd, "hello world\n", 12);
  sockets::close(connfd);
  loop.quit();
}

void handleClose(){}

int main() {
  Logger::setLogLevel(Logger::TRACE);
  InetAddress localAddr(12001);

  listenSocket.bindAddress(localAddr);
  listenSocket.listen();
  Channel channel(&loop, listenSocket.fd());
  channel.setReadCallback(handleRead);
  channel.enableReading();

  LOG_INFO<<"main thread start loop";
  loop.loop();
  channel.disableAll();
  channel.remove();

  return 0;
}