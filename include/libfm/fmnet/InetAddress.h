//
// Created by Ye-zixiao on 2021/8/8.
//

#ifndef LIBFM_INCLUDE_LIBFM_FMNET_INETADDRESS_H_
#define LIBFM_INCLUDE_LIBFM_FMNET_INETADDRESS_H_

#include <string>
#include <arpa/inet.h>

namespace fm::net {

class InetAddress {
 public:
  explicit InetAddress(uint16_t port = 0, bool loopBackOnly = false, bool ipv6 = false);
  explicit InetAddress(std::string ip, uint16_t port, bool ipv6 = false);

  explicit InetAddress(const struct sockaddr_in &addr4) : addr_(addr4) {}
  explicit InetAddress(const struct sockaddr_in6 &addr6) : addr6_(addr6) {}

  uint16_t port() const;
  sa_family_t family() const { return addr_.sin_family; }

  std::string toIpStr() const;
  std::string toIpPortStr() const;

  void setSockAddrInet6(const struct sockaddr_in6 &addr6) { addr6_ = addr6; }
  const struct sockaddr *getSockAddr() const;

  uint16_t portNetEdian() const { return addr_.sin_port; }
  uint32_t ipv4NetEdian() const;

  // 不推荐使用
  static bool resolve(const std::string& hostname, InetAddress *result);

 private:
  union {
    // 这两个套接字结构的前面部分是相同的，因此
    // IPv4而言复用IPv6的地址结构完全没有问题
    struct sockaddr_in addr_;
    struct sockaddr_in6 addr6_;
  };
};

} // namespace fm::net

#endif //LIBFM_INCLUDE_LIBFM_FMNET_INETADDRESS_H_
