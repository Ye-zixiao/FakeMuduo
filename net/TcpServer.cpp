//
// Created by Ye-zixiao on 2021/4/8.
//

#include "TcpServer.h"

#include "../base/Logging.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "EventLoopThreadPool.h"

#include <cstdio>

using namespace fm;
using namespace fm::net;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr,
					 std::string name, Option option)
	: loop_((assert(loop), loop)),
	  ipPort_(listenAddr.toIpStr()),
	  name_(std::move(name)),
	  acceptor_(std::make_unique<Acceptor>(loop, listenAddr, option == kNoReusePort)),
	  threadPool_(std::make_shared<EventLoopThreadPool>(loop_, name_)),
	  connectionCallback_(defaultConnectionCallback),
	  messageCallback_(defaultMessageCallback),
	  start_(false),
	  nextConnId_(1) {
  acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer() {
  loop_->assertInLoopThread();
  LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

  for (auto &item:connections_) {
	TcpConnectionPtr conn(item.second);
	item.second.reset();
	conn->getLoop()->runInLoop(
		std::bind(&TcpConnection::connectDestroyed, conn));
  }
}

void TcpServer::setThreadNum(int numThreads) {
  assert(0 <= numThreads);
  threadPool_->setThreadNum(numThreads);
}

void TcpServer::start() {
  if (start_.exchange(true) == false) {
	threadPool_->start();

	assert(!acceptor_->isListening());
	loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
  }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
  loop_->assertInLoopThread();
  EventLoop *ioLoop = threadPool_->getNextLoop();
  char buf[64];
  snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_++);
  std::string connName(name_ + buf);
  LOG_INFO << "TcpServer::newConnection [" << name_
		   << "] - new connection [" << connName
		   << "] from " << peerAddr.toIpPortStr()
		   << " ioLoop threadId " << ioLoop->threadId();

  // 为客户的连接套接字创建一个TcpConnection类对象，并注册到某一个
  // sub-Reactor线程中的事件循环中进行监听管理，并为其注册相应的回调函数
  InetAddress localAddr(sockets::getLocalAddr(sockfd));
  TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop,
														  connName,
														  sockfd,
														  localAddr,
														  peerAddr);
  connections_[connName] = conn;
  conn->setConnectionCallback(connectionCallback_);
  conn->setMessageCallback(messageCallback_);
  conn->setWriteCompleteCallback(writeCompleteCallback_);
  conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1));
  // 通过runInLoop()的方法将连接相关的通道注册到Robin-Round法
  // 选出的sub-Reactor线程的事件循环之中
  ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
  LOG_TRACE << "remove Connection " << conn->peerAddr().toIpPortStr();
  // 这个loop是main Reactor的事件循环
  loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
  loop_->assertInLoopThread();
  LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
		   << "] - connection " << conn->name();

  size_t n = connections_.erase(conn->name());
  assert(n == 1);
  EventLoop *ioLoop = conn->getLoop();
  ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}