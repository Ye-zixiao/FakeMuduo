//
// Created by Ye-zixiao on 2021/4/7.
//

#include "InetAddress.h"

#include <netdb.h>
#include <cassert>

#include "../base/Logging.h"
#include "Endian.h"

using namespace fm;
using namespace fm::net;

InetAddress::InetAddress(uint16_t port, bool loopBackOnly, bool ipv6) {
  memset(&addr6_, 0, sizeof(addr6_));
  if (ipv6) {
	addr6_.sin6_family = AF_INET6;
	addr6_.sin6_addr = loopBackOnly ? in6addr_loopback : in6addr_any;
	addr6_.sin6_port = sockets::hostToNetwork16(port);
  } else {
	addr_.sin_family = AF_INET;
	addr_.sin_addr.s_addr = loopBackOnly ? INADDR_LOOPBACK : INADDR_ANY;
	addr_.sin_port = sockets::hostToNetwork16(port);
  }
}

InetAddress::InetAddress(std::string ip, uint16_t port, bool ipv6) {
  if (ipv6 || strchr(ip.c_str(), ':')) {
	memset(&addr6_, 0, sizeof(addr6_));
	sockets::fromIpPortStr(ip.c_str(), port, &addr6_);
  } else {
	memset(&addr_, 0, sizeof(addr_));
	sockets::fromIpPortStr(ip.c_str(), port, &addr_);
  }
}

std::string InetAddress::toIpStr() const {
  char buf[INET6_ADDRSTRLEN]{};
  sockets::toIpStr(getSockAddr(), buf, sizeof(buf));
  return buf;
}

std::string InetAddress::toIpPortStr() const {
  char buf[64]{};
  sockets::toIpPortStr(getSockAddr(), buf, sizeof(buf));
  return buf;
}

uint16_t InetAddress::port() const {
  return sockets::networkToHost16(addr_.sin_port);
}

uint32_t InetAddress::ipv4NetEdian() const {
  assert(addr_.sin_family == AF_INET);
  return addr_.sin_addr.s_addr;
}

static thread_local char resolveBuffer_ts[64 * 1024];

// 建议不要使用
bool InetAddress::resolve(std::string hostname, InetAddress *result) {
  assert(result != nullptr);
  struct hostent hent{};
  struct hostent *he{};
  int herrno = 0;

  // 是否能使用getaddrinfo_r代替？
  int ret = gethostbyname_r(hostname.c_str(), &hent, resolveBuffer_ts,
							sizeof(resolveBuffer_ts), &he, &herrno);
  if (ret == 0 && he != nullptr) {
	assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
	result->addr_.sin_addr = *reinterpret_cast<struct in_addr *>(he->h_addr_list);
	return true;
  } else {
	if (ret)
	  LOG_SYSERR << "InetAddress::resolve";
	return false;
  }
}