//
// Created by Ye-zixiao on 2021/4/8.
//

#ifndef FAKEMUDUO_NET_TCPSERVER_H_
#define FAKEMUDUO_NET_TCPSERVER_H_

#include <unordered_map>
#include <atomic>
#include <string>
#include <memory>

#include "../base/noncoapyable.h"
#include "TcpConnection.h"
#include "InetAddress.h"

namespace fm {
namespace net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer : private noncopyable {
 public:
  enum Option { kNoReusePort, kReusePort };

  TcpServer(EventLoop *loop,
			const InetAddress &listenAddr,
			std::string name,
			Option option = kNoReusePort);
  ~TcpServer();

  const std::string &ipPortStr() const { return ipPort_; }
  const std::string &name() const { return name_; }
  EventLoop *getLoop() const { return loop_; }

  void setThreadNum(int numThreads);

  void start();

  void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
  void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }

 private:
  void newConnection(int sockfd, const InetAddress &peerAddr);
  void removeConnection(const TcpConnectionPtr &conn);
  void removeConnectionInLoop(const TcpConnectionPtr &conn);

 private:
  using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

  EventLoop *loop_;                                  // main-Reactor的事件循环
  const std::string ipPort_;
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_;               // 接收器Acceptor
  std::shared_ptr<EventLoopThreadPool> threadPool_;  // sub-Reactor线程池，其实可以定义为独一指针

  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;

  std::atomic_bool start_;
  int nextConnId_;
  ConnectionMap connections_;
};

} // namespace net
} // namespace fm

#endif //FAKEMUDUO_NET_TCPSERVER_H_
