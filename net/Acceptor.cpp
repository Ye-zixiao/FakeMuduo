//
// Created by Ye-zixiao on 2021/4/8.
//

#include "net/Acceptor.h"
#include <cerrno>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include "libfm/fmutil/Log.h"
#include "libfm/fmnet/InetAddress.h"
#include "libfm/fmnet/EventLoop.h"
#include "net/SocketsOps.h"

using namespace fm;
using namespace fm::net;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reusePort)
    : loop_(loop),
      accept_socket_(sockets::createNonblockingSocketOrDie(listenAddr.family())),
      accept_channel_(loop, accept_socket_.fd()),
      listening_(false),
      idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
  assert(accept_socket_.fd() >= 0);
  accept_socket_.setReuseAddr(true);
  accept_socket_.setReusePort(reusePort);
  accept_socket_.bindAddress(listenAddr);
  accept_channel_.setReadCallback(
      [this](time::TimeStamp t) { this->handleRead(t); });
}

Acceptor::~Acceptor() {
  accept_channel_.disableAll();
  accept_channel_.remove();
  ::close(idle_fd_);
}

void Acceptor::setNewConnectionCallback(const NewConnectionCallback &cb) {
  new_connection_callback_ = cb;
}

void Acceptor::listen() {
  loop_->assertInLoopThread();
  listening_ = true;
  accept_socket_.listen();
  accept_channel_.enableReading();
}

void Acceptor::handleRead(time::TimeStamp) {
  loop_->assertInLoopThread();
  InetAddress peerAddr;
  int connfd = accept_socket_.accept(&peerAddr);
  if (connfd >= 0) {
    LOG_DEBUG << "Acceptor accept a new connection, fd = " << connfd;
    if (new_connection_callback_)
      // 调用TcpServer的newConnection()成员函数
      new_connection_callback_(connfd, peerAddr);
    else
      ::close(connfd);
  } else {
    LOG_ERROR << "in Acceptor::handleRead";
    if (errno == EMFILE) {
      ::close(idle_fd_);
      idle_fd_ = ::accept(accept_socket_.fd(), nullptr, nullptr);
      ::close(idle_fd_);
      idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}