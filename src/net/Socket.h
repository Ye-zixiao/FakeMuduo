//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef FAKEMUDUO_NET_SOCKET_H_
#define FAKEMUDUO_NET_SOCKET_H_

#include "../base/noncoapyable.h"

namespace fm {
namespace net {

class InetAddress;

class Socket : private noncopyable {
 public:
  explicit Socket(int sockfd)
	  : sockfd_(sockfd) {}

  ~Socket();

  int fd() const { return sockfd_; }

  void bindAddress(const InetAddress &localAddr);
  void listen();
  int accept(InetAddress *peerAddr);

  void shutdownWrite();

  void setTcpNoDelay(bool on);
  void setReuseAddr(bool on);
  void setReusePort(bool on);
  void setKeepAlive(bool on);

 private:
  const int sockfd_;
};

} // namespace net
} // namespace fm

#endif //FAKEMUDUO_NET_SOCKET_H_
