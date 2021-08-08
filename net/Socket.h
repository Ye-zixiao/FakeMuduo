//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef LIBFM_NET_SOCKET_H_
#define LIBFM_NET_SOCKET_H_

namespace fm::net {

class InetAddress;

class Socket {
 public:
  explicit Socket(int sockfd)
      : sockfd_(sockfd) {}

  ~Socket();

  Socket(const Socket &) = delete;
  Socket &operator=(const Socket &) = delete;

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

} // namespace fm::net

#endif //LIBFM_NET_SOCKET_H_
