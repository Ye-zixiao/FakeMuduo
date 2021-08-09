//
// Created by Ye-zixiao on 2021/8/8.
//

#ifndef DOCS_OLDFILES_LIBFM_INCLUDE_LIBFM_FMNET_TCPCLIENT_H_
#define DOCS_OLDFILES_LIBFM_INCLUDE_LIBFM_FMNET_TCPCLIENT_H_

#include <string>
#include <mutex>
#include "libfm/fmnet/TcpConnection.h"

namespace fm::net {

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient {
 public:
  TcpClient(EventLoop *loop,
            const InetAddress &address,
            std::string name);
  ~TcpClient();

  TcpClient(const TcpClient &) = delete;
  TcpClient &operator=(const TcpClient &) = delete;

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const;
  EventLoop *getLoop() const { return loop_; }
  bool retry() const { return retry_; }
  void enableRetry() { retry_ = true; }
  std::string_view name() const { return name_; }

  void setConnectionCallback(ConnectionCallback cb);
  void setMessageCallback(MessageCallback cb);
  void setWriteCompleteCallback(WriteCompleteCallback cb);

 private:
  void newConnection(int sockfd);
  void removeConnection(const TcpConnectionPtr &conn);

 private:
  EventLoop *loop_;
  ConnectorPtr connector_;
  const std::string name_;

  ConnectionCallback connection_callback_;
  MessageCallback message_callback_;
  WriteCompleteCallback write_complete_callback_;

  bool retry_;    // 连接断开之后是否重新连接
  bool connect_;  // 这个没搞明白
  int next_connid_;
  mutable std::mutex mutex_;
  TcpConnectionPtr connection_;
};

inline void TcpClient::setConnectionCallback(ConnectionCallback cb) {
  this->connection_callback_ = std::move(cb);
}

inline void TcpClient::setMessageCallback(MessageCallback cb) {
  this->message_callback_ = std::move(cb);
}

inline void TcpClient::setWriteCompleteCallback(WriteCompleteCallback cb) {
  this->write_complete_callback_ = std::move(cb);
}

} // namespace fm::net

#endif //DOCS_OLDFILES_LIBFM_INCLUDE_LIBFM_FMNET_TCPCLIENT_H_
