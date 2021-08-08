//
// Created by Ye-zixiao on 2021/4/8.
//

#include "libfm/fmnet/TcpServer.h"
#include "libfm/fmnet/EventLoop.h"
#include "libfm/fmnet/EventLoopThread.h"
#include "libfm/fmutil/Log.h"
#include "net/EventLoopThreadPool.h"
#include "net/SocketsOps.h"
#include "net/Acceptor.h"

using namespace fm;
using namespace fm::net;

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr,
                     std::string name, Option option)
    : loop_((assert(loop), loop)),
      ip_port_(listenAddr.toIpStr()),
      name_(std::move(name)),
      acceptor_(std::make_unique<Acceptor>(loop, listenAddr, option == kNoReusePort)),
      thread_pool_(std::make_shared<EventLoopThreadPool>(loop_, name_)),
      connection_callback_(defaultConnectionCallback),
      message_callback_(defaultMessageCallback),
      start_(false),
      next_conn_id_(1) {
  acceptor_->setNewConnectionCallback(
      [this](int sockfd, const InetAddress &peer_addr) {
        this->newConnection(sockfd, peer_addr);
      });
}

TcpServer::~TcpServer() {
  loop_->assertInLoopThread();
  LOG_DEBUG << "TcpServer::~TcpServer [" << name_ << "] destructing";

  for (auto &item:connections_) {
    TcpConnectionPtr conn(item.second);
    item.second.reset();
    conn->getLoop()->runInLoop(
        [conn] { conn->connectDestroyed(); });
  }
}

void TcpServer::setThreadNum(int numThreads) {
  assert(0 <= numThreads);
  thread_pool_->setThreadNum(numThreads);
}

void TcpServer::start() {
  if (!start_.exchange(true)) {
    thread_pool_->start();

    assert(!acceptor_->isListening());
    loop_->runInLoop([acceptor = acceptor_.get()] { acceptor->listen(); });
  }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
  loop_->assertInLoopThread();
  EventLoop *ioLoop = thread_pool_->getNextLoop();
  char buf[64];
  snprintf(buf, sizeof(buf), "-%s#%d", ip_port_.c_str(), next_conn_id_++);
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
  conn->setConnectionCallback(connection_callback_);
  conn->setMessageCallback(message_callback_);
  conn->setWriteCompleteCallback(write_complete_callback_);
  conn->setCloseCallback([this](const TcpConnectionPtr &c) { this->removeConnection(c); });
  // 通过runInLoop()的方法将连接相关的通道注册到Robin-Round法
  // 选出的sub-Reactor线程的事件循环之中
  ioLoop->runInLoop([conn] { conn->connectEstablished(); });
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
  LOG_INFO << "remove Connection " << conn->peerAddr().toIpPortStr();
  // 这个loop是main Reactor的事件循环
  loop_->runInLoop([this, conn] { this->removeConnectionInLoop(conn); });
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
  loop_->assertInLoopThread();
  LOG_DEBUG << "TcpServer::removeConnectionInLoop [" << name_
            << "] - connection " << conn->name();

  size_t n = connections_.erase(std::string(conn->name()));
  assert(n == 1);
  (void)n;

  EventLoop *ioLoop = conn->getLoop();
  // std::bind(&TcpConnection::connectDestroyed, conn)会在内部拷贝处一个
  // 共享指针TcpConnectionPtr指向TcpConnection对象，只有当这个function在sub-Reactor
  // 线程之中执行完毕之后才会是的TcpConnection对象自动析构销毁（引用计数从1变成0）
  ioLoop->queueInLoop([conn] { conn->connectDestroyed(); });
  LOG_DEBUG << "conn use_count: " << conn.use_count(); // 2
}