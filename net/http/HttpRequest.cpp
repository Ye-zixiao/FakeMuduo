//
// Created by Ye-zixiao on 2021/4/30.
//

#include "HttpRequest.h"

using namespace fm;
using namespace fm::net;
using namespace fm::net::http;

HttpRequest::HttpRequest(HttpRequest &&rhs) noexcept
    : method_(rhs.method_),
      version_(rhs.version_),
      path_(std::move(rhs.path_)),
      query_(std::move(rhs.query_)),
      receivedTime_(rhs.receivedTime_),
      headers_(std::move(rhs.headers_)) {
  rhs.method_ = kInvalidMethod;
  rhs.version_ = kUnknownVersion;
}

HttpRequest &HttpRequest::operator=(HttpRequest &&rhs) noexcept {
  method_ = rhs.method_;
  rhs.method_ = kInvalidMethod;
  version_ = rhs.version_;
  rhs.version_ = kUnknownVersion;
  path_ = std::move(rhs.path_);
  query_ = std::move(rhs.query_);
  receivedTime_ = rhs.receivedTime_;
  headers_ = std::move(rhs.headers_);
  return *this;
}

bool HttpRequest::setMethod(const char *start, const char *end) {
  std::string m(start, end);
  if (m == "GET") method_ = kGet;
  else if (m == "POST") method_ = kPost;
  else if (m == "HEAD") method_ = kHead;
  else if (m == "PUT") method_ = kPut;
  else if (m == "DELETE") method_ = kDelete;
  else method_ = kInvalidMethod;
  return method_ != kInvalidMethod;
}

std::string HttpRequest::methodToString() const {
  std::string result("UNKNOWN");
  switch (method_) {
    case kGet:result = "GET";
      break;
    case kPost:result = "POST";
      break;
    case kPut:result = "PUT";
      break;
    case kDelete:result = "DELETE";
      break;
    case kHead:result = "HEAD";
      break;
    default: break;
  }
  return result;
}

void HttpRequest::addHeader(const char *start, const char *colon, const char *end) {
  std::string field(start, colon++);
  while (colon < end && isspace(*colon))
    ++colon;
  std::string value(colon, end);
  while (!value.empty() && isspace(value[value.size() - 1]))
    value.resize(value.size() - 1);
  headers_[field] = value;
}

std::string HttpRequest::getHeader(const std::string &field) const {
  auto iter = headers_.find(field);
  if (iter != headers_.end())
    return iter->second;
  return {};
}

void HttpRequest::swap(HttpRequest &rhs) {
  std::swap(method_, rhs.method_);
  std::swap(version_, rhs.version_);
  std::swap(path_, rhs.path_);
  std::swap(query_, rhs.query_);
  std::swap(receivedTime_, rhs.receivedTime_);
  std::swap(headers_, rhs.headers_);
}

void HttpRequest::clear() {
  method_ = kInvalidMethod;
  version_ = kUnknownVersion;
  path_.clear();
  query_.clear();
  headers_.clear();
}