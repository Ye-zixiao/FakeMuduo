//
// Created by Ye-zixiao on 2021/8/8.
//

#ifndef DOCS_OLDFILES_LIBFM_NET_CONNECTOR_H_
#define DOCS_OLDFILES_LIBFM_NET_CONNECTOR_H_

#include <functional>
#include <memory>
#include "libfm/fmnet/InetAddress.h"

namespace fm::net {

class Channel;
class EventLoop;

// 连接器Connector在Tcp客户端编程中的地位等同于Tcp服务端编程中的接收器Acceptor
class Connector : public std::enable_shared_from_this<Connector> {
 public:
  using NewConnectionCallback = std::function<void(int sockfd)>;

  Connector(EventLoop *loop, const InetAddress &address);
  ~Connector();

  void start();
  void restart(); // 由IO线程负责调用
  void stop();

  void setNewConnectionCallback(const NewConnectionCallback &cb);
  const InetAddress &serverAddress() const { return server_address_; }

 private:
  enum class State { kDisconnected, kConnecting, kConnected };
  void setState(State state) { state_ = state; }

  void startInLoop();
  void stopInLoop();

  void connect();
  void connecting(int sockfd);

  void handleWrite();
  void handleError();

  void retry(int sockfd);
  int removeAndResetChannel();
  void resetChannel();

 private:
  static constexpr int kMaxRetryDelayMs = 30 * 1000;
  static constexpr int kInitRetryDelayMs = 500;

  InetAddress server_address_;
  EventLoop *loop_;
  bool connect_;
  State state_;
  std::unique_ptr<Channel> channel_;
  NewConnectionCallback new_connection_callback_;
  int retry_delay_ms_;
};

inline void Connector::setNewConnectionCallback(const NewConnectionCallback &cb) {
  this->new_connection_callback_ = cb;
}

} // namespace fm::net

#endif //DOCS_OLDFILES_LIBFM_NET_CONNECTOR_H_
