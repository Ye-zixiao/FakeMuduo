//
// Created by Ye-zixiao on 2021/8/7.
//

#ifndef LIBFM_UTIL_BASELOGGER_H_
#define LIBFM_UTIL_BASELOGGER_H_

namespace fm::log {

class LogLine;

class BaseLogger {
 public:
  virtual ~BaseLogger() = default;
  virtual void add(LogLine &&log_line) = 0;
};

} // namespace fm::log

#endif //LIBFM_UTIL_BASELOGGER_H_
