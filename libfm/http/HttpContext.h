//
// Created by Ye-zixiao on 2021/4/30.
//

#ifndef LIBFM_HTTP_HTTPCONTEXT_H_
#define LIBFM_HTTP_HTTPCONTEXT_H_

#include "libfm/base/copyable.h"
#include "libfm/http/HttpRequest.h"

namespace fm::net {

class Buffer;

namespace http {

class HttpContext : public copyable {
 public:
  enum HttpRequestParseState { kParseRequestLine, kParseHeaders, kParseBody, kAllDone };

  HttpContext() : state_(kParseRequestLine), request_() {}
  HttpContext(const HttpContext &rhs) = default;
  HttpContext(HttpContext &&rhs) noexcept = default;
  HttpContext &operator=(const HttpContext &rhs) = default;
  HttpContext &operator=(HttpContext &&rhs) = default;

  bool parseRequest(Buffer *buffer, time::Timestamp receivedTime);

  const HttpRequest &request() const { return request_; }
  HttpRequest &request() { return request_; }

  bool isAllDone() const { return state_ == kAllDone; }

  void reset() { state_ = kParseRequestLine, request_.clear(); }

 private:
  bool processRequestLine(const char *begin, const char *end);

 private:
  HttpRequestParseState state_;
  HttpRequest request_;
};

} // namespace http
} // namespace fm

#endif //LIBFMO_HTTP_HTTPCONTEXT_H_
