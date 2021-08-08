//
// Created by Ye-zixiao on 2021/8/8.
//

#ifndef LIBFM_INCLUDE_LIBFM_FMNET_TCPCONNECTIONI_H_
#define LIBFM_INCLUDE_LIBFM_FMNET_TCPCONNECTIONI_H_

#include <memory>
#include <any>
#include "libfm/fmnet/InetAddress.h"
#include "libfm/fmnet/Callback.h"
#include "libfm/fmnet/Buffer.h"

namespace fm {

namespace time {
class TimeStamp;
} // namespace fm::time

namespace net {

class Channel;
class Socket;
class EventLoop;

class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop *loop,
                std::string name,
                int sockfd,
                const InetAddress &localAddr,
                const InetAddress &peerAddr);

  ~TcpConnection();

  TcpConnection(const TcpConnection &) = delete;
  TcpConnection &operator=(const TcpConnection &) = delete;

  EventLoop *getLoop() const { return loop_; }
  std::string_view name() const { return name_; }
  const InetAddress &localAddr() const { return local_addr_; }
  const InetAddress &peerAddr() const { return peer_addr_; }
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

  void setConnectionCallback(const ConnectionCallback &cb) { connection_callback_ = cb; }
  void setMessageCallback(const MessageCallback &cb) { message_callback_ = cb; }
  void setWriteCompleteCallback(const WriteCompleteCallback &cb) { write_complete_callback_ = cb; }
  void setHighWaterMarkCallback(const HighWaterMarkCallback &cb) { high_water_mark_callback_ = cb; }
  void setCloseCallback(const CloseCallback &cb) { close_callback_ = cb; }

  void setUserContext(const std::any &context) { user_context_ = context; }
  void setUserContext(std::any &&context) { user_context_ = std::move(context); }
  const std::any &getUserContext() const { return user_context_; }
  std::any *getMutableUserContext() { return &user_context_; }

  // 调试使用
  Buffer *inputBuffer() { return &input_buffer_; }
  Buffer *outputBuffer() { return &output_buffer_; }

  void connectEstablished();
  void connectDestroyed();

 private:
  enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

  void handleRead(time::TimeStamp receiveTime);
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
  const std::string name_;
  EventLoop *loop_;
  StateE state_;
  bool reading_;

  // channel_是一个独一指针，而Channel内部的实现却无不在将自己的
  // 原始指针交给别的类使用，这样使用方式必须要求注意Channel的使用
  std::unique_ptr<Socket> socket_;
  std::unique_ptr<Channel> channel_;
  const InetAddress local_addr_;
  const InetAddress peer_addr_;

  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  WriteCompleteCallback write_complete_callback_;
  HighWaterMarkCallback high_water_mark_callback_;
  CloseCallback close_callback_;

  size_t high_water_mark_;
  Buffer input_buffer_;
  Buffer output_buffer_;
  std::any user_context_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

} // namespace fm::net

} // namespace fm

#endif //LIBFM_INCLUDE_LIBFM_FMNET_TCPCONNECTIONI_H_
