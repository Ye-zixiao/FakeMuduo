//
// Created by Ye-zixiao on 2021/4/30.
//

#ifndef FAKEMUDUO_HTTP_HTTPRESPONSE_H_
#define FAKEMUDUO_HTTP_HTTPRESPONSE_H_

#include <unordered_map>
#include <string>

#include "../base/copyable.h"
#include "HttpBase.h"

namespace fm {

namespace net {

class Buffer;

namespace http {

class HttpResponse : public copyable {
 public:
  enum HttpStatusCode {
    kInvalidStatus,
    k200OK = 200,
    k301MovedPermanently = 301,
    k400BadRequest = 400,
    k404NotFound = 404
  };

  explicit HttpResponse(bool close)
      : version_(kUnknownVersion),
        statusCode_(kInvalidStatus),
        closeConnection_(close) {}

  void setVersion(Version version) { version_ = version; }
  Version getVersion() const { return version_; }

  void setStatusCode(HttpStatusCode code) { statusCode_ = code; }
  void setStatusMessage(const std::string &message) { statusMessage_ = message; }

  void setCloseConnection(bool on) { closeConnection_ = on; }
  bool isCloseConnection() const { return closeConnection_; }

  void setContentType(const std::string &contentType);

  void addHeader(const std::string &field, const std::string &value);

  void setBody(const std::string &body) { body_ = body; }

  void appendToBuffer(net::Buffer *outBuffer) const;

 private:
  Version version_;
  HttpStatusCode statusCode_;
  std::string statusMessage_;
  std::unordered_map<std::string, std::string> headers_;
  bool closeConnection_;
  std::string body_;
};

} // namespace http

} // namespace net

} // namespace fm

#endif //FAKEMUDUO_HTTP_HTTPRESPONSE_H_
