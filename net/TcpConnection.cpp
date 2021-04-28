//
// Created by Ye-zixiao on 2021/4/8.
//

#include "TcpConnection.h"

#include "../base/Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

#include <cerrno>
#include <cassert>

using namespace fm;
using namespace fm::net;

void fm::net::defaultConnectionCallback(const TcpConnectionPtr &conn) {
  LOG_TRACE << conn->localAddr().toIpPortStr() << " -> "
			<< conn->peerAddr().toIpPortStr() << " is "
			<< (conn->isConnected() ? "UP" : "DOWN");
}

void fm::net::defaultMessageCallback(const TcpConnectionPtr &conn,
									 Buffer *buf, TimeStamp) {
  buf->retrieveAll();
}

TcpConnection::TcpConnection(EventLoop *loop,
							 std::string name,
							 int sockfd,
							 const InetAddress &localAddr,
							 const InetAddress &peerAddr)
	: loop_(loop),
	  name_(std::move(name)),
	  state_(kConnecting),
	  reading_(true),// why true?
	  socket_(std::make_unique<Socket>(sockfd)),
	  channel_(std::make_unique<Channel>(loop, sockfd)),
	  localAddr_(localAddr),
	  peerAddr_(peerAddr),
	  highWaterMark_(1024 * 1024 * 64) {
  // 将与此连接相关的回调函数进行注册，要求分派器（这里也即EventLoop）在读/写准备
  // 事件发生的时候回调通道Chanel上注册的各类事件回调函数，而后者会回调到TcpConnection
  // 这里来！分别调用handleRead()、handleWrite()、handleClose()等
  channel_->setReadCallback(std::bind(&TcpConnection::handleRead, this, _1));
  channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
  channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
  channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));
  LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
			<< this << " fd = " << sockfd;
  socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
  // 由于使用了智能指针，所以TcpConnection在析构的时候不需要太多的处理
  LOG_DEBUG << "TcpConnection:dtor[" << name_ << "] at " << this
			<< " fd = " << channel_->fd() << " state = " << stateToString();
  assert(state_ == kDisconnected);
}

void TcpConnection::send(const void *msg, int len) {
  send(std::string(static_cast<const char *>(msg), len));
}

void TcpConnection::send(const std::string &msg) {
  if (state_ == kConnected) {
	if (loop_->isInLoopThread())
	  sendInLoop(msg);
	else {
	  void (TcpConnection::*fp)(const std::string &) =&TcpConnection::send;
	  loop_->runInLoop(std::bind(fp, this, msg));
	}
  }
}

void TcpConnection::send(Buffer *buf) {
  if (state_ == kConnected) {
	if (loop_->isInLoopThread()) {
	  sendInLoop(buf->peek(), buf->readableBytes());
	  buf->retrieveAll();
	} else {
	  void (TcpConnection::*fp)(const std::string &msg) =&TcpConnection::send;
	  // 感觉存在一些无意义的拷贝
	  loop_->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()));
	}
  }
}

void TcpConnection::sendInLoop(const std::string &msg) {
  sendInLoop(msg.data(), msg.size());
}

void TcpConnection::sendInLoop(const void *msg, size_t len) {
  loop_->assertInLoopThread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool faultError = false;
  if (state_ == kDisconnected) {
	LOG_WARN << "disconnected, give up writing";
	return;
  }

  // 若当前TcpConnection相关的频道不关心可写事件或者输出缓冲区中没有数据，那么直接
  // I/O线程执行套接字输出操作；否则将这个用户给定的数据放到输出缓冲区中，等下一次可写
  // 事件触发的时候自动回调handleWrite()处理。
  // 	为什么不直接写？因为该频道之所以先前关注可写感兴趣事件正说明该套接字内核缓冲区
  // 空间不足嘛！若此时继续直接write(sockfd,...)也没什么用，所以还不如直接放到输出
  // 缓冲区中，等待回调handleWrite()的时候自动处理。
  if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
	nwrote = sockets::write(channel_->fd(), msg, len);
	LOG_TRACE << "IOLoop write " << nwrote << " bytes to client";
	if (nwrote >= 0) {
	  remaining = len - nwrote;
	  if (remaining == 0 && writeCompleteCallback_)
		// 不直接这次出回调，而是放在doPendingFunctors()函数中处理
		loop_->queueInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
	} else {
	  nwrote = 0;
	  if (errno != EWOULDBLOCK) {
		// 那么问题来了，发生错误时TcpConnection这些对象怎么办？
		LOG_SYSERR << "TcpConnection::sendInLoop";
		if (errno == EPIPE || errno == ECONNRESET)
		  faultError = true;
	  }
	}
  }

  assert(remaining <= len);
  if (!faultError && remaining > 0) {
	size_t oldLen = outputBuffer_.readableBytes();
	// 输出缓冲区中的数据量超出了高水位线
	if (oldLen + remaining >= highWaterMark_ &&
		oldLen < highWaterMark_ &&
		highWaterMarkCallback_)
	  loop_->queueInLoop(std::bind(highWaterMarkCallback_,
								   shared_from_this(),
								   oldLen + remaining));

	// 将剩下未发送的数据放到输出缓冲区中，待以后通过handleWrite()来完成发送
	outputBuffer_.append(static_cast<const char *>(msg) + nwrote, remaining);
	if (!channel_->isWriting())
	  channel_->enableWriting();
  }
}

void TcpConnection::shutdown() {
  if (state_ == kConnected) {
	setState(kDisconnecting);
	loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
  }
}

void TcpConnection::shutdownInLoop() {
  loop_->assertInLoopThread();
  if (!channel_->isWriting())
	socket_->shutdownWrite();
}

void TcpConnection::forceClose() {
  if (state_ == kConnected || state_ == kDisconnecting) {
	setState(kDisconnecting);
	loop_->queueInLoop(std::bind(&TcpConnection::forceCloseInLoop,
								 shared_from_this()));
  }
}

void TcpConnection::forceCloseInLoop() {
  loop_->assertInLoopThread();
  if (state_ == kConnected || state_ == kDisconnecting)
	handleClose();
}

void TcpConnection::forceCloseWithDelay(double seconds) {
  if (state_ == kConnected || state_ == kDisconnecting) {
	setState(kDisconnecting);
	// ...
  }
}

void TcpConnection::setTcpNoDelay(bool on) {
  socket_->setTcpNoDelay(on);
}

void TcpConnection::startRead() {
  // startRead()是提供给用户层编程者使用的，连接建立之初不会调用这个
  loop_->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop() {
  loop_->assertInLoopThread();
  if (!reading_ || !channel_->isReading()) {
	channel_->enableReading();
	reading_ = true;
  }
}

void TcpConnection::stopRead() {
  loop_->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop() {
  loop_->assertInLoopThread();
  if (reading_ || channel_->isReading()) {
	channel_->disableReading();
	reading_ = false;
  }
}

void TcpConnection::connectEstablished() {
  loop_->assertInLoopThread();
  assert(state_ == kConnecting);
  setState(kConnected);
  channel_->tie(shared_from_this());
  // 连接建立之初就直接让EventLoop开启了对Tcp连接可读事件的注册
  channel_->enableReading();

  connectionCallback_(shared_from_this());
}

void TcpConnection::connectDestroyed() {
  loop_->assertInLoopThread();

  /* 连接关闭的时候主要是需要做两个重要的工作：
   * 1、将连接信息从TcpServer的ConnectionMap中剔除，这个工作需要
   * 	main-Reactor所在的I/O线程来完成
   * 2、将通道信息从Poller中的ChannelMap中剔除，这个工作只需要原先
   * 	通道所注册的sub-Reacotr所在的I/O线程来完成
   * */

  // 一般来说这个if语句内部的东西并不会被执行
  if (state_ == kConnected) {
	setState(kDisconnected);
	channel_->disableAll();

	// 调用下用户注册的连接建立/关闭的回调函数
	connectionCallback_(shared_from_this());
  }
  channel_->remove();
}

void TcpConnection::handleRead(TimeStamp receiveTime) {
  loop_->assertInLoopThread();
  int savedErrno = 0;
  ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
  if (n > 0)
	messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
  else if (n == 0)
	handleClose();
  else {
	errno = savedErrno;
	LOG_SYSERR << "TcpConnection::handleRead";
	handleError();
  }
}

void TcpConnection::handleWrite() {
  loop_->assertInLoopThread();
  if (channel_->isWriting()) {
	ssize_t n = sockets::write(channel_->fd(),
							   outputBuffer_.peek(),
							   outputBuffer_.readableBytes());
	if (n > 0) {
	  outputBuffer_.retrieve(n);
	  if (outputBuffer_.readableBytes() == 0) {
		channel_->disableWriting();
		if (writeCompleteCallback_)
		  // 写完成回调函数可能对该套接字进行写半部关
		  loop_->queueInLoop(std::bind(writeCompleteCallback_,
									   shared_from_this()));
		if (state_ == kDisconnecting)
		  shutdownInLoop();
	  }
	}
  } else {
	LOG_SYSERR << "TcpConnection::handleWrite";
  }
}

void TcpConnection::handleClose() {
  loop_->assertInLoopThread();
  LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
  assert(state_ == kConnected || state_ == kDisconnecting);
  setState(kDisconnected);
  channel_->disableAll();

  TcpConnectionPtr guardThis(shared_from_this());
  connectionCallback_(guardThis);  // 调用用户注册的onConnection回调函数

  closeCallback_(guardThis); // 调用TcpServer中的removeConnection()方法
  LOG_TRACE << " guard use_count: " << guardThis.use_count(); // 4
}

void TcpConnection::handleError() {
  int err = sockets::getSocketError(channel_->fd());
  LOG_ERROR << "TcpConnection::handleError [" << name_
			<< "] - SO_ERROR = " << err << " " << strerror_ts(err);
}

const char *TcpConnection::stateToString() const {
  switch (state_) {
	case kConnecting:return "kConnecting";
	case kConnected:return "kConnected";
	case kDisconnecting:return "kDisconneting";
	case kDisconnected:return "kDisconnected";
	default:return "unknown state";
  }
}