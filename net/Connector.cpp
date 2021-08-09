//
// Created by Ye-zixiao on 2021/8/8.
//

#include "net/Connector.h"
#include <cerrno>
#include <cassert>
#include "libfm/fmutil/Log.h"
#include "libfm/fmnet/Channel.h"
#include "libfm/fmnet/EventLoop.h"
#include "libfm/fmnet/TimerId.h"
#include "net/SocketsOps.h"

namespace fm::net {

Connector::Connector(EventLoop *loop, const InetAddress &address)
    : server_address_(address),
      loop_(loop),
      connect_(false),
      state_(State::kDisconnected),
      retry_delay_ms_(kInitRetryDelayMs) {
  LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector() {
  LOG_DEBUG << "dtor[" << this << "]";
  assert(!channel_);
}

void Connector::start() {
  connect_ = true;
  loop_->runInLoop([this] { this->startInLoop(); });
}

void Connector::startInLoop() {
  loop_->assertInLoopThread();
  assert(state_ == State::kDisconnected);
  if (connect_)
    connect();
  else
    LOG_DEBUG << "do not connect";
}

void Connector::stop() {
  connect_ = false;
  loop_->queueInLoop([this] { stopInLoop(); });
}

void Connector::stopInLoop() {
  loop_->assertInLoopThread();
  if (state_ == State::kConnecting) {
    setState(State::kDisconnected);
    int sockfd = removeAndResetChannel();
    retry(sockfd);
  }
}

void Connector::connect() {
  // 创建连接套接字并尝试以非阻塞方式进行连接
  int sockfd = sockets::createNonblockingSocketOrDie(server_address_.family());
  int ret = sockets::connect(sockfd, server_address_.getSockAddr());
  int saved_errno = (ret == 0) ? 0 : errno;
  switch (saved_errno) {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sockfd);  // 连接未失败，但还没连接成功，则向EventLoop注册回调事件
      break;
    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sockfd);       // 重新连接
      break;
    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_ERROR << "connect error in Connector::startInLoop " << saved_errno;
      sockets::close(sockfd);
      break;
    default:
      LOG_ERROR << "Unexpected error in Connector::startInLoop" << saved_errno;
      sockets::close(sockfd);
      break;
  }
}

void Connector::restart() {
  loop_->assertInLoopThread();
  setState(State::kDisconnected);
  retry_delay_ms_ = kInitRetryDelayMs;
  connect_ = true;
  startInLoop();
}

void Connector::connecting(int sockfd) {
  setState(State::kConnecting);
  assert(!channel_);
  channel_ = std::make_unique<Channel>(loop_, sockfd);
  channel_->setWriteCallback([this] { this->handleWrite(); });
  channel_->setErrorCallback([this] { this->handleError(); });
  channel_->enableWriting();
}

int Connector::removeAndResetChannel() {
  channel_->disableAll();
  channel_->remove();
  int sockfd = channel_->fd();
  loop_->queueInLoop([this] { this->resetChannel(); });
  return sockfd;
}

void Connector::resetChannel() {
  channel_.reset();
}

void Connector::handleWrite() {
  LOG_DEBUG << "Connector::handleWrite "
            << *reinterpret_cast<uint8_t *>(&state_);

  if (state_ == State::kConnecting) { // 连接套接字正处于“正在连接状态”
    int sockfd = removeAndResetChannel(); // 无论连接成功与否，都要将这个套接字的注册事件删除
    int err = sockets::getSocketError(sockfd);
    if (err) {
      LOG_WARN << "Connector::handleWrite - SO_ERROR = " << err;
      retry(sockfd);
    } else if (sockets::isSelfConnect(sockfd)) {
      LOG_WARN << "Connector::handleWrite - Self connect";
      retry(sockfd);
    } else {
      setState(State::kConnected);
      if (connect_)
        // 连接成功则会回调用户的连接成功回调函数，同时用户会得到相应的套接字描述符
        new_connection_callback_(sockfd);
      else
        sockets::close(sockfd);
    }
  } else {
    assert(state_ == State::kDisconnected);
  }
}

void Connector::handleError() {
  LOG_ERROR << "Connector::handleError state = "
            << *reinterpret_cast<uint8_t *>(&state_);

  if (state_ == State::kConnecting) {
    int sockfd = removeAndResetChannel();
    int err = sockets::getSocketError(sockfd);
    LOG_ERROR << "SO_ERROR = " << err;
    retry(sockfd);
  }
}

void Connector::retry(int sockfd) {
  sockets::close(sockfd);
  setState(State::kDisconnected);
  if (connect_) {
    LOG_INFO << "Connector::retry - Retry connecting to " << server_address_.toIpPortStr()
             << " in " << retry_delay_ms_ << " milliseconds";
    loop_->runAfter(std::chrono::microseconds(retry_delay_ms_),
                    [st = shared_from_this()] { st->startInLoop(); });
    retry_delay_ms_ = std::min(retry_delay_ms_ * 2, kMaxRetryDelayMs);
  } else {
    LOG_DEBUG << "do not connect";
  }
}

} // namespace fm::net