//
// Created by Ye-zixiao on 2021/4/30.
//

#include "libfm/http/HttpContext.h"

#include <algorithm>

#include "libfm/net/Buffer.h"

using namespace fm;
using namespace fm::net;
using namespace fm::net::http;

bool HttpContext::parseRequest(Buffer *buffer, time::Timestamp receivedTime) {
  bool ok = true, hasMore = true;
  while (hasMore) {
    switch (state_) {
      // 1、解析http请求行
      case kParseRequestLine: {
        const char *crlf = buffer->findCRLF();
        if (crlf) {
          ok = processRequestLine(buffer->peek(), crlf);
          if (ok) {
            request_.setReceivedTime(receivedTime);
            buffer->retrieveUntil(crlf + 2);
            state_ = kParseHeaders;
          } else {
            ok = false;
          }
        } else {
          hasMore = false;
        }
        break;
      }
      // 2、解析http首部字段
      case kParseHeaders: {
        const char *crlf = buffer->findCRLF();
        if (crlf) {
          const char *colon = std::find(buffer->peek(), crlf, ':');
          if (colon != crlf) {
            // 处理普通首部字段
            request_.addHeader(buffer->peek(), colon, crlf);
          } else {
            // 处理到\r\n空行
            state_ = kAllDone;
            hasMore = false;
          }
          buffer->retrieveUntil(crlf + 2);
        } else {
          hasMore = false;
        }
        break;
      }
      // 3、解析http实体体部分
      case kParseBody:
      default:break;
    }
  }
  return ok;
}

bool HttpContext::processRequestLine(const char *begin, const char *end) {
  bool succeed = false;
  const char *start = begin;
  const char *space = std::find(begin, end, ' ');
  // 1、解析http请求行方法字段
  if (space != end && request_.setMethod(start, space)) {
    start = space + 1;
    space = std::find(start, end, ' ');
    // 2、解析http请求行路径字段
    if (space != end) {
      const char *question = std::find(start, space, '?');
      if (question != space) {
        request_.setPath(start, question);
        request_.setQuery(question, space);
      } else {
        request_.setPath(start, space);
      }

      start = space + 1;
      succeed = end - start == 8 && std::equal(start, end - 3, "HTTP/")
          && *(end - 2) == '.';
      // 3、解析http请求行协议字段
      if (succeed) {
        if (*(end - 3) == '1' && *(end - 1) == '1')
          request_.setVersion(kHttp11);
        else if (*(end - 3) == '1' && *(end - 1) == '0')
          request_.setVersion(kHttp10);
        else if (*(end - 3) == '2' && *(end - 1) == '0')
          request_.setVersion(kHttp20);
        else
          succeed = false;
      }
    }
  }
  return succeed;
}