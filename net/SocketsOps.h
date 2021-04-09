//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef FAKEMUDUO_NET_SOCKETSOPS_H_
#define FAKEMUDUO_NET_SOCKETSOPS_H_

#include <arpa/inet.h>

namespace fm {

namespace net {

namespace sockets {

int createNonblockingSocketOrDie(sa_family_t family);

// 从sockaddr_in和sockaddr_in6的结构字段上看，因此前者是可以复用后者，
// 即：无论是IPv4地址还是IPv6的地址都使用IPv6的套接字地址结构体进行保存
int connect(int sockfd, const struct sockaddr *addr);
void bindOrDie(int sockfd, const struct sockaddr *addr);
void listenOrDie(int sockfd);
int accept(int sockfd, struct sockaddr_in6 *addr);

ssize_t read(int sockfd, void *buf, size_t size);
ssize_t readv(int sockfd, const struct iovec *iov, int iovcnt);
ssize_t write(int sockfd, const void *buf, size_t size);
ssize_t writev(int sockfd, const iovec *iov, int iovcnt);

void close(int sockfd);
void shutdownWrite(int sockfd);

void toIpPortStr(const struct sockaddr *addr, char *buf, size_t size);
void toIpStr(const struct sockaddr *addr, char *buf, size_t size);
void fromIpPortStr(const char *ip, uint16_t port, struct sockaddr_in *addr);
void fromIpPortStr(const char *ip, uint16_t port, struct sockaddr_in6 *addr);

int getSocketError(int sockfd);

const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr);
const struct sockaddr *sockaddr_cast(const struct sockaddr_in6 *addr);
struct sockaddr *sockaddr_cast(struct sockaddr_in *addr);
struct sockaddr *sockaddr_cast(struct sockaddr_in6 *addr);
const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr);
const struct sockaddr_in6 *sockaddr_in6_cast(const struct sockaddr *addr);
struct sockaddr_in *sockaddr_in_cast(struct sockaddr *addr);
struct sockaddr_in6 *sockaddr_in6_cast(struct sockaddr *addr);

struct sockaddr_in6 getLocalAddr(int sockfd);
struct sockaddr_in6 getPeerAddr(int sockfd);

} // namespace sockets

} // namespace net

} // namespace fm


#endif //FAKEMUDUO_NET_SOCKETSOPS_H_
