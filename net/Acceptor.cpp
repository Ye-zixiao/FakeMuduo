//
// Created by Ye-zixiao on 2021/4/8.
//

#include "Acceptor.h"

#include <cerrno>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>

#include "../base/Logging.h"
#include "InetAddress.h"
#include "EventLoop.h"

using namespace fm;
using namespace fm::net;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reusePort)
	: loop_(loop),
	  acceptSocket_(sockets::createNonblockingSocketOrDie(listenAddr.family())),
	  acceptChannel_(loop, acceptSocket_.fd()),
	  listening_(false),
	  idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
  assert(acceptSocket_.fd() >= 0);
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.setReusePort(reusePort);
  acceptSocket_.bindAddress(listenAddr);
  acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
  acceptChannel_.disableAll();
  acceptChannel_.remove();
  ::close(idleFd_);
}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback &cb) {
  newConnectionCallback_ = cb;
}

void Acceptor::listen() {
  loop_->assertInLoopThread();
  listening_ = true;
  acceptSocket_.listen();
  acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
  loop_->assertInLoopThread();
  InetAddress peerAddr;
  int connfd = acceptSocket_.accept(&peerAddr);
  if (connfd >= 0) {
	LOG_TRACE << "Acceptor accept a new connection, fd = " << connfd;
	if (newConnectionCallback_)
	  // 调用TcpServer的newConnection()成员函数
	  newConnectionCallback_(connfd, peerAddr);
	else
	  ::close(connfd);
  } else {
	LOG_SYSERR << "in Acceptor::handleRead";
	if (errno == EMFILE) {
	  ::close(idleFd_);
	  idleFd_ = ::accept(acceptSocket_.fd(), nullptr, nullptr);
	  ::close(idleFd_);
	  idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
	}
  }
}