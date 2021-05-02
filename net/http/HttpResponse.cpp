//
// Created by Ye-zixiao on 2021/4/30.
//

#include "HttpResponse.h"

#include <cstdio>

#include "../../base/Logging.h"
#include "../Buffer.h"

using namespace fm;
using namespace fm::net;
using namespace fm::net::http;

void HttpResponse::setContentType(const std::string &contentType) {
  addHeader("Content-Type", contentType);
}

void HttpResponse::addHeader(const std::string &field, const std::string &value) {
  headers_[field] = value;
}

static inline const char *versionToStr(Version version) {
  assert(version != kUnknownVersion);
  const char *verStr = "";
  switch (version) {
    case kHttp10:verStr = "HTTP/1.0";
      break;
    case kHttp11:verStr = "HTTP/1.1";
      break;
    case kHttp20:verStr = "HTTP/2.0";
      break;
    default:break;
  }
  return verStr;
}

void HttpResponse::appendToBuffer(Buffer *outBuffer) const {
  char buf[32]{};
  snprintf(buf, sizeof(buf), "%s %d ", versionToStr(version_), statusCode_);
  outBuffer->append(buf);
  outBuffer->append(statusMessage_);
  outBuffer->append("\r\n");

  if (closeConnection_) {
    outBuffer->append("Connection: close\r\n");
  } else {
    snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
    outBuffer->append(buf);
    outBuffer->append("Connection: keep-alive\r\n");
  }

  for (const auto &header:headers_) {
    outBuffer->append(header.first);
    outBuffer->append(": ");
    outBuffer->append(header.second);
    outBuffer->append("\r\n");
  }
  outBuffer->append("\r\n");
  outBuffer->append(body_);
}