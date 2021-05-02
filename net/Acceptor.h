//
// Created by Ye-zixiao on 2021/4/8.
//

#ifndef FAKEMUDUO_NET_ACCEPTOR_H_
#define FAKEMUDUO_NET_ACCEPTOR_H_

#include <functional>

#include "../base/noncoapyable.h"
#include "Channel.h"
#include "Socket.h"

namespace fm {
namespace net {

class EventLoop;
class InetAddress;

class Acceptor : private noncopyable {
 public:
  // 新建连接套接字的回调函数的第一个参数是套接字描述符
  using NewConnectionCallback = std::function<void(int, const InetAddress &)>;

  Acceptor(EventLoop *loop, const InetAddress &listenAddr, bool reusePort);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback &cb);

  void listen();

  bool isListening() const { return listening_; }

 private:
  void handleRead();

 private:
  EventLoop *loop_;
  Socket acceptSocket_;
  Channel acceptChannel_;
  NewConnectionCallback newConnectionCallback_;
  bool listening_;
  int idleFd_;
};

} // namespace net
} // namespace fm

#endif //FAKEMUDUO_NET_ACCEPTOR_H_
