//
// Created by Ye-zixiao on 2021/8/8.
//

#ifndef LIBFM_INCLUDE_LIBFM_FMNET_TCPSERVER_H_
#define LIBFM_INCLUDE_LIBFM_FMNET_TCPSERVER_H_

#include <unordered_map>
#include <atomic>
#include <string>
#include <memory>
#include "libfm/fmnet/TcpConnection.h"
#include "libfm/fmnet/InetAddress.h"

namespace fm::net {

class Acceptor;
class EventLoop;
class EventLoopThreadPool;

class TcpServer {
 public:
  enum Option { kNoReusePort, kReusePort };

  TcpServer(EventLoop *loop,
            const InetAddress &peer_addr,
            std::string name,
            Option option = kNoReusePort);
  ~TcpServer();

  TcpServer(const TcpServer &) = delete;
  TcpServer &operator=(const TcpServer &) = delete;

  std::string_view ipPortStr() const { return ip_port_; }
  std::string_view name() const { return name_; }
  EventLoop *getLoop() const { return loop_; }

  void setThreadNum(int numThreads);

  void start();

  void setConnectionCallback(const ConnectionCallback &cb) { connection_callback_ = cb; }
  void setMessageCallback(const MessageCallback &cb) { message_callback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback &cb) { write_complete_callback_ = cb; }

 private:
  void newConnection(int sockfd, const InetAddress &peerAddr);
  void removeConnection(const TcpConnectionPtr &conn);
  void removeConnectionInLoop(const TcpConnectionPtr &conn);

 private:
  using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

  EventLoop *loop_;                                  // main-Reactor的事件循环
  const std::string ip_port_;
  const std::string name_;
  std::unique_ptr<Acceptor> acceptor_;               // 接收器Acceptor
  std::shared_ptr<EventLoopThreadPool> thread_pool_;  // sub-Reactor线程池，其实可以定义为独一指针

  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  WriteCompleteCallback write_complete_callback_;

  std::atomic_bool start_;
  int next_conn_id_;
  ConnectionMap connections_;
};

} // namespace fm::net

#endif //LIBFM_INCLUDE_LIBFM_FMNET_TCPSERVER_H_
