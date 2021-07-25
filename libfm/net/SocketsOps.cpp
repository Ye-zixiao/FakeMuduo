//
// Created by Ye-zixiao on 2021/4/7.
//

#include "libfm/net/SocketsOps.h"
#include <sys/uio.h>
#include <unistd.h>
#include <cerrno>
#include "libfm/base/Logging.h"
#include "libfm/net/Endian.h"

/***
 *  这里特别提醒一句：
 *  从sockaddr_in和sockaddr_in6的结构字段上看，因此前者是可以复用后者，
 *  即：无论是IPv4地址还是IPv6的地址都使用IPv6的套接字地址结构体进行保存。
 *  而下面的很多函数也正是如此处理之！
 */

using namespace fm::net;

int sockets::createNonblockingSocketOrDie(sa_family_t family) {
  int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK |
      SOCK_CLOEXEC, IPPROTO_TCP);
  if (sockfd < 0)
    LOG_SYSFATAL << "sockets::createNonblockingSocketOrDie";
  return sockfd;
}

int sockets::connect(int sockfd, const struct sockaddr *addr) {
  return ::connect(sockfd, addr, sizeof(struct sockaddr_in6));
}

void sockets::bindOrDie(int sockfd, const struct sockaddr *addr) {
  int ret = ::bind(sockfd, addr, sizeof(struct sockaddr_in6));
  if (ret < 0)
    LOG_SYSFATAL << "sockets::bindOrDie";
}

void sockets::listenOrDie(int sockfd) {
  int ret = ::listen(sockfd, SOMAXCONN);
  if (ret < 0)
    LOG_SYSFATAL << "sockets::listenOrDie";
}

int sockets::accept(int sockfd, struct sockaddr_in6 *addr) {
  auto addrlen = static_cast<socklen_t>(sizeof(struct sockaddr_in6));
  int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connfd < 0) {
    int savedErrno = errno;
    LOG_SYSERR << "sockets::accept";
    switch (savedErrno) {
      case EAGAIN:
      case ECONNABORTED:
      case EINTR:
      case EPERM:
      case EPROTO:
      case EMFILE: errno = savedErrno;
        break;
      case EBADF:
      case EFAULT:
      case EINVAL:
      case ENOBUFS:
      case ENOMEM:
      case ENOTSOCK:
      case EOPNOTSUPP: LOG_FATAL << "unexpected error of ::accept " << savedErrno;
        break;
      default: LOG_FATAL << "unknown error of ::accept " << savedErrno;
        break;
    }
  }
  return connfd;
}

ssize_t sockets::read(int sockfd, void *buf, size_t size) {
  return ::read(sockfd, buf, size);
}

ssize_t sockets::readv(int sockfd, const struct iovec *iov, int iovcnt) {
  return ::readv(sockfd, iov, iovcnt);
}

ssize_t sockets::write(int sockfd, const void *buf, size_t size) {
  return ::write(sockfd, buf, size);
}

ssize_t sockets::writev(int sockfd, const iovec *iov, int iovcnt) {
  return ::writev(sockfd, iov, iovcnt);
}

void sockets::close(int sockfd) {
  if (::close(sockfd) < 0)
    LOG_SYSERR << "sockets::close";
}

void sockets::shutdownWrite(int sockfd) {
  if (::shutdown(sockfd, SHUT_WR) < 0)
    LOG_SYSERR << "sockets::shutdownWrite";
}

void sockets::toIpPortStr(const struct sockaddr *addr, char *buf, size_t size) {
  if (addr->sa_family == AF_INET6) {
    buf[0] = '[';
    toIpStr(addr, buf + 1, size - 1);
    size_t endPos = ::strlen(buf);
    const struct sockaddr_in6 *addr6 = sockaddr_in6_cast(addr);
    uint16_t port = networkToHost16(addr6->sin6_port);
    snprintf(buf + endPos, size - endPos, "]:%u", port);
  } else if (addr->sa_family == AF_INET) {
    toIpStr(addr, buf, size);
    size_t endPos = ::strlen(buf);
    const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
    uint16_t port = networkToHost16(addr4->sin_port);
    snprintf(buf + endPos, size - endPos, ":%u", port);
  }
}

void sockets::toIpStr(const struct sockaddr *addr, char *buf, size_t size) {
  if (addr->sa_family == AF_INET) {
    const struct sockaddr_in *addr4 = sockaddr_in_cast(addr);
    ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
  } else if (addr->sa_family == AF_INET6) {
    const struct sockaddr_in6 *addr6 = sockaddr_in6_cast(addr);
    ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
  }
}

void sockets::fromIpPortStr(const char *ip, uint16_t port, struct sockaddr_in *addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    LOG_SYSERR << "sockets::fromIpPortStr";
}

void sockets::fromIpPortStr(const char *ip, uint16_t port, struct sockaddr_in6 *addr) {
  addr->sin6_family = AF_INET6;
  addr->sin6_port = hostToNetwork16(port);
  if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
    LOG_SYSERR << "sockets::fromIpPortStr";
}

struct sockaddr_in6 sockets::getLocalAddr(int sockfd) {
  struct sockaddr_in6 localAddr{};
  auto addrlen = static_cast<socklen_t>(sizeof(localAddr));
  if (::getsockname(sockfd, sockaddr_cast(&localAddr), &addrlen) < 0)
    LOG_SYSERR << "sockets::getLocalAddr";
  return localAddr;
}

struct sockaddr_in6 sockets::getPeerAddr(int sockfd) {
  struct sockaddr_in6 peerAddr{};
  auto addrlen = static_cast<socklen_t>(sizeof(peerAddr));
  if (::getpeername(sockfd, sockaddr_cast(&peerAddr), &addrlen) < 0)
    LOG_SYSERR << "sockets::getPeerAddr";
  return peerAddr;
}

int sockets::getSocketError(int sockfd) {
  int optval;
  auto optlen = static_cast<socklen_t>(sizeof(optval));
  if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    return errno;
  return optval;
}

const struct sockaddr *sockets::sockaddr_cast(const struct sockaddr_in *addr) {
  return static_cast<const struct sockaddr *>(static_cast<const void *>(addr));
}

const struct sockaddr *sockets::sockaddr_cast(const struct sockaddr_in6 *addr) {
  return static_cast<const struct sockaddr *>(static_cast<const void *>(addr));
}

struct sockaddr *sockets::sockaddr_cast(struct sockaddr_in *addr) {
  return static_cast<struct sockaddr *>(static_cast<void *>(addr));
}

struct sockaddr *sockets::sockaddr_cast(struct sockaddr_in6 *addr) {
  return static_cast<struct sockaddr *>(static_cast<void *>(addr));
}

const struct sockaddr_in *sockets::sockaddr_in_cast(const struct sockaddr *addr) {
  return static_cast<const struct sockaddr_in *>(static_cast<const void *>(addr));
}

const struct sockaddr_in6 *sockets::sockaddr_in6_cast(const struct sockaddr *addr) {
  return static_cast<const struct sockaddr_in6 *>(static_cast<const void *>(addr));
}

struct sockaddr_in *sockets::sockaddr_in_cast(struct sockaddr *addr) {
  return static_cast<struct sockaddr_in *>(static_cast<void *>(addr));
}

struct sockaddr_in6 *sockets::sockaddr_in6_cast(struct sockaddr *addr) {
  return static_cast<struct sockaddr_in6 *>(static_cast<void *>(addr));
}