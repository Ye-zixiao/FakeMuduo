//
// Created by Ye-zixiao on 2021/4/30.
//

#ifndef LIBFM_HTTP_HTTPREQUEST_H_
#define LIBFM_HTTP_HTTPREQUEST_H_

#include <unordered_map>
#include <string>

#include "libfm/base/copyable.h"
#include "libfm/base/Timestamp.h"
#include "libfm/http/HttpBase.h"

namespace fm::net::http {

class HttpRequest : public copyable {
 public:
  enum Method { kInvalidMethod, kGet, kPost, kHead, kPut, kDelete };

  using HeaderMap = std::unordered_map<std::string, std::string>;

  HttpRequest() : method_(kInvalidMethod), version_(kUnknownVersion) {}

  HttpRequest(const HttpRequest &rhs) = default;
  HttpRequest(HttpRequest &&rhs) noexcept;
  HttpRequest &operator=(const HttpRequest &rhs) = default;
  HttpRequest &operator=(HttpRequest &&rhs) noexcept;

  bool setMethod(const char *start, const char *end);
  Method getMethod() const { return method_; }
  std::string methodToString() const;

  void setPath(const char *start, const char *end) { path_.assign(start, end); }
  const std::string &path() const { return path_; }

  void setVersion(Version v) { version_ = v; }
  Version getVersion() const { return version_; }

  void setQuery(const char *start, const char *end) { query_.assign(start, end); }
  const std::string &query() const { return query_; }

  void setReceivedTime(time::Timestamp t) { receivedTime_ = t; }
  const time::Timestamp &receivedTime() const { return receivedTime_; }

  void addHeader(const char *start, const char *colon, const char *end);
  std::string getHeader(const std::string &field) const;
  const HeaderMap &headers() const { return headers_; }

  void swap(HttpRequest &rhs);
  void clear();

 private:
  Method method_;
  Version version_;
  std::string path_;
  std::string query_;
  time::Timestamp receivedTime_;
  HeaderMap headers_;
};

} // namespace fm::net::http

#endif //LIBFM_HTTP_HTTPREQUEST_H_
