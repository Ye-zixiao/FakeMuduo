//
// Created by Ye-zixiao on 2021/8/8.
//

#ifndef OLD_LIBFM_INCLUDE_LIBFM_FMNET_HTTP_HTTPRESPONSE_H_
#define OLD_LIBFM_INCLUDE_LIBFM_FMNET_HTTP_HTTPRESPONSE_H_

#include <unordered_map>
#include <string>
#include "libfm/fmnet/http/HttpBase.h"

namespace fm::net {

class Buffer;

namespace http {

class HttpResponse {
 public:
  enum HttpStatusCode {
    kInvalidStatus = 0,
    k200OK = 200,
    k301MovedPermanently = 301,
    k400BadRequest = 400,
    k404NotFound = 404
  };

  using HeaderMap = std::unordered_map<std::string, std::string>;

  explicit HttpResponse(bool close)
      : version_(kUnknownVersion),
        statusCode_(kInvalidStatus),
        closeConnection_(close) {}

  HttpResponse(const HttpResponse &) = default;
  HttpResponse &operator=(const HttpResponse &) = default;

  void setVersion(Version version) { version_ = version; }
  Version getVersion() const { return version_; }

  void setStatusCode(HttpStatusCode code) { statusCode_ = code; }
  void setStatusMessage(const std::string &message) { statusMessage_ = message; }

  void setCloseConnection(bool on) { closeConnection_ = on; }
  bool isCloseConnection() const { return closeConnection_; }

  void setContentType(const std::string &contentType);

  void addHeader(const std::string &field, const std::string &value);

  void setBody(const std::string &body) { body_ = body; }

  void appendToBuffer(Buffer *outBuffer) const;

 private:
  Version version_;
  HttpStatusCode statusCode_;
  std::string statusMessage_;
  HeaderMap headers_;
  bool closeConnection_;
  std::string body_;
};

} // namespace fm::net::http

} // namespace fm

#endif //OLD_LIBFM_INCLUDE_LIBFM_FMNET_HTTP_HTTPRESPONSE_H_
