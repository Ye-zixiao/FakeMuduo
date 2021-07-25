//
// Created by Ye-zixiao on 2021/4/8.
//

#ifndef LIBFM_NET_ACCEPTOR_H_
#define LIBFM_NET_ACCEPTOR_H_

#include <functional>
#include "libfm/base/NonCopyable.h"
#include "libfm/net/Channel.h"
#include "libfm/net/Socket.h"

namespace fm::net {

class EventLoop;
class InetAddress;

class Acceptor : private NonCopyable {
 public:
  // 新建连接套接字的回调函数的第一个参数是套接字描述符
  using NewConnectionCallback = std::function<void(int, const InetAddress &)>;

  Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reusePort);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback &cb);

  void listen();

  bool isListening() const { return listening_; }

 private:
  void handleRead(time::Timestamp);

 private:
  EventLoop *loop_;
  Socket accept_socket_;
  Channel accept_channel_;
  NewConnectionCallback new_connection_callback_;
  bool listening_;
  int idle_fd_;
};

} // namespace fm::net

#endif //LIBFM_NET_ACCEPTOR_H_
