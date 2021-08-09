//
// Created by Ye-zixiao on 2021/8/8.
//

#include "libfm/fmnet/TcpClient.h"
#include "libfm/fmnet/EventLoop.h"
#include "libfm/fmutil/Log.h"
#include "net/SocketsOps.h"
#include "net/Connector.h"

namespace fm::net {

TcpClient::TcpClient(EventLoop *loop,
                     const InetAddress &address,
                     std::string name)
    : loop_(loop),
      connector_(std::make_shared<Connector>(loop, address)),
      name_(std::move(name)),
      connection_callback_(defaultConnectionCallback),
      message_callback_(defaultMessageCallback),
      retry_(false),
      connect_(true),
      next_connid_(1) {
  connector_->setNewConnectionCallback(
      [this](int sockfd) { this->newConnection(sockfd); });
  LOG_INFO << "TcpClient::TcpClient[" << name_
           << "] - connector " << connector_.get();
}

TcpClient::~TcpClient() {
  LOG_INFO << "TcpClient::~TcpClient[" << name_
           << "] - connetor " << connector_.get();
  TcpConnectionPtr conn;
  bool unique = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    unique = connection_.unique();
    conn = connection_;
  }
  if (conn) {
    assert(loop_ == conn->getLoop());
    CloseCallback cb = [loop = loop_](const TcpConnectionPtr &cn) {
      loop->queueInLoop([&cn] { cn->connectDestroyed(); });
    };
    loop_->runInLoop([conn, cb] { conn->setCloseCallback(cb); });
    if (unique)
      conn->forceClose();
  } else {
    connector_->stop();
//    loop_->runAfter(1,[])
  }
}

void TcpClient::connect() {
  LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
           << connector_->serverAddress().toIpPortStr();
  connect_ = true;
  connector_->start();
}

void TcpClient::disconnect() {
  connect_ = false;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (connection_)
      connection_->shutdown();
  }
}

void TcpClient::stop() {
  connect_ = false;
  connector_->stop();
}

TcpConnectionPtr TcpClient::connection() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return connection_;
}

void TcpClient::newConnection(int sockfd) {
  loop_->assertInLoopThread();
  InetAddress peer_address(sockets::getPeerAddr(sockfd));
  char buf[32]{};
  snprintf(buf, sizeof(buf), ":%s#%d",
           peer_address.toIpPortStr().c_str(), next_connid_++);
  std::string conn_name = name_ + buf;

  InetAddress local_address(sockets::getLocalAddr(sockfd));
  TcpConnectionPtr conn(std::make_shared<TcpConnection>(loop_,
                                                        conn_name,
                                                        sockfd,
                                                        local_address,
                                                        peer_address));
  conn->setConnectionCallback(connection_callback_);
  conn->setMessageCallback(message_callback_);
  conn->setWriteCompleteCallback(write_complete_callback_);
  conn->setCloseCallback([this](const auto &cn) { this->removeConnection(cn); });
  {
    std::lock_guard<std::mutex> lock(mutex_);
    connection_ = conn;
  }
  conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn) {
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());

  {
    std::lock_guard<std::mutex> lock(mutex_);
    assert(connection_ == conn);
    connection_.reset();
  }

  loop_->queueInLoop([conn] { conn->connectDestroyed(); });
  if (retry_ && connect_) {
    // 连接断开之后尝试重新连接
    LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconneting to"
             << connector_->serverAddress().toIpPortStr();
    connector_->restart();
  }
}

} // namespace fm::net
