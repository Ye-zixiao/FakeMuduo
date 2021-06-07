//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef LIBFM_NET_SOCKETSOPS_H_
#define LIBFM_NET_SOCKETSOPS_H_

#include <arpa/inet.h>

namespace fm::net::sockets {

int createNonblockingSocketOrDie(sa_family_t family);

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

struct sockaddr_in6 getLocalAddr(int sockfd);
struct sockaddr_in6 getPeerAddr(int sockfd);

int getSocketError(int sockfd);

const struct sockaddr *sockaddr_cast(const struct sockaddr_in *addr);
const struct sockaddr *sockaddr_cast(const struct sockaddr_in6 *addr);
struct sockaddr *sockaddr_cast(struct sockaddr_in *addr);
struct sockaddr *sockaddr_cast(struct sockaddr_in6 *addr);

const struct sockaddr_in *sockaddr_in_cast(const struct sockaddr *addr);
const struct sockaddr_in6 *sockaddr_in6_cast(const struct sockaddr *addr);
struct sockaddr_in *sockaddr_in_cast(struct sockaddr *addr);
struct sockaddr_in6 *sockaddr_in6_cast(struct sockaddr *addr);


} // namespace fm::net::sockets


#endif //LIBFM_NET_SOCKETSOPS_H_
