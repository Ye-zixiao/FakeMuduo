//
// Created by Ye-zixiao on 2021/4/8.
//

#ifndef FAKEMUDUO_NET_TCPCONNECTION_H_
#define FAKEMUDUO_NET_TCPCONNECTION_H_

#include "../base/noncoapyable.h"
#include "InetAddress.h"
#include "Callback.h"
#include "Buffer.h"

#include <memory>
#include <any>

namespace fm {

namespace net {

class Channel;
class Socket;
class EventLoop;

class TcpConnection : private noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop *loop,
                std::string name,
                int sockfd,
                const InetAddress &localAddr,
                const InetAddress &peerAddr);

  ~TcpConnection();

  EventLoop *getLoop() const { return loop_; }
  std::string name() const { return name_; }
  const InetAddress &localAddr() const { return localAddr_; }
  const InetAddress &peerAddr() const { return peerAddr_; }
  bool isConnected() const { return state_ == kConnected; }
  bool isDisconnected() const { return state_ == kDisconnected; }

  void send(const void *msg, int len);
  void send(const std::string &msg);
  void send(Buffer *buf);

  void shutdown();
  void forceClose();
  void forceCloseWithDelay(double seconds);
  void setTcpNoDelay(bool on);

  void startRead();
  void stopRead();
  bool isReading() const { return reading_; }

  void setConnectionCallback(const ConnectionCallback &cb) { connectionCallback_ = cb; }
  void setMessageCallback(const MessageCallback &cb) { messageCallback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback &cb) { writeCompleteCallback_ = cb; }
  void setHighWaterMarkCallback(const HighWaterMarkCallback &cb) { highWaterMarkCallback_ = cb; }
  void setCloseCallback(const CloseCallback &cb) { closeCallback_ = cb; }

  void setUserContext(const std::any &context) { userContext_ = context; }
  const std::any &getUserContext() const { return userContext_; }
  std::any *getMutableUserContext() { return &userContext_; }

  // 调试使用
  Buffer *inputBuffer() { return &inputBuffer_; }
  Buffer *outputBuffer() { return &outputBuffer_; }

  void connectEstablished();
  void connectDestroyed();

 private:
  enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

  void handleRead(TimeStamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();

  void sendInLoop(const std::string &msg);
  void sendInLoop(const void *msg, size_t len);
  void shutdownInLoop();
  void forceCloseInLoop();
  void startReadInLoop();
  void stopReadInLoop();
  void setState(StateE s) { state_ = s; }
  const char *stateToString() const;

 private:
  EventLoop *loop_;
  const std::string name_;
  StateE state_;
  bool reading_;

  // channel_是一个独一指针，而Channel内部的实现却无不在将自己的原始指针
  // 交给别的类使用，这样使用方式必须要求注意Channel的使用
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  const InetAddress localAddr_;
  const InetAddress peerAddr_;

  ConnectionCallback connectionCallback_;
  MessageCallback messageCallback_;
  WriteCompleteCallback writeCompleteCallback_;
  HighWaterMarkCallback highWaterMarkCallback_;
  CloseCallback closeCallback_;

  size_t highWaterMark_;
  Buffer inputBuffer_;
  Buffer outputBuffer_;
  std::any userContext_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

} // namespace net

} // namespace fm

#endif //FAKEMUDUO_NET_TCPCONNECTION_H_