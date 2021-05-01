//
// Created by Ye-zixiao on 2021/4/30.
//

#ifndef FAKEMUDUO_HTTP_HTTPREQUEST_H_
#define FAKEMUDUO_HTTP_HTTPREQUEST_H_

#include <unordered_map>
#include <string>

#include "../base/copyable.h"
#include "../base/TimeStamp.h"
#include "HttpBase.h"

namespace fm {

namespace net {

namespace http {

class HttpRequest : public copyable {
 public:
  enum Method { kInvalidMethod, kGet, kPost, kHead, kPut, kDelete };

  HttpRequest() : method_(kInvalidMethod), version_(kUnknownVersion) {}

  bool setMethod(const char *start, const char *end);
  Method getMethod() const { return method_; }
  std::string methodToString() const;

  void setPath(const char *start, const char *end) { path_.assign(start, end); }
  const std::string &path() const { return path_; }

  void setVersion(Version v) { version_ = v; }
  Version getVersion() const { return version_; }

  void setQuery(const char *start, const char *end) { query_.assign(start, end); }
  const std::string &query() const { return query_; }

  void setReceivedTime(TimeStamp t) { receivedTime_ = t; }
  const TimeStamp &receivedTime() const { return receivedTime_; }

  void addHeader(const char *start, const char *colon, const char *end);
  std::string getHeader(const std::string &field) const;
  const std::unordered_map<std::string, std::string> &headers() const { return headers_; }

  void swap(HttpRequest &rhs);
  void clear();

 private:
  Method method_;
  Version version_;
  std::string path_;
  std::string query_;
  TimeStamp receivedTime_;
  std::unordered_map<std::string, std::string> headers_;
};

} // namespace http

} // namespace net

} // namespace fm

#endif //FAKEMUDUO_HTTP_HTTPREQUEST_H_
