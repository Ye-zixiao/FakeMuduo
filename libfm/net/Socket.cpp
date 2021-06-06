//
// Created by Ye-zixiao on 2021/4/7.
//

#include "libfm/net/Socket.h"

#include <netinet/tcp.h>

#include "libfm/base/Logging.h"
#include "libfm/net/SocketsOps.h"
#include "libfm/net/InetAddress.h"

using namespace fm;
using namespace fm::net;

Socket::~Socket() {
  sockets::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localAddr) {
  sockets::bindOrDie(sockfd_, localAddr.getSockAddr());
}

void Socket::listen() {
  sockets::listenOrDie(sockfd_);
}

int Socket::accept(InetAddress *peerAddr) {
  struct sockaddr_in6 addr{};
  int connfd = sockets::accept(sockfd_, &addr);
  if (connfd >= 0)
	peerAddr->setSockAddrInet6(addr);
  return connfd;
}

void Socket::shutdownWrite() {
  sockets::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on) {
  const int on_off = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
			   &on_off, static_cast<socklen_t>(on_off));
}

void Socket::setReuseAddr(bool on) {
  const int on_off = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
			   &on_off, static_cast<socklen_t>(sizeof(on_off)));
}

void Socket::setReusePort(bool on) {
  const int on_off = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
			   &on_off, static_cast<socklen_t>(sizeof(on_off)));
}

void Socket::setKeepAlive(bool on) {
  const int on_off = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
			   &on_off, static_cast<socklen_t>(sizeof(on_off)));
}