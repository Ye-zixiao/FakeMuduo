//
// Created by Ye-zixiao on 2021/8/7.
//

#ifndef LIBFM_UTIL_STDLOGGER_H_
#define LIBFM_UTIL_STDLOGGER_H_

#include <atomic>
#include "util/BaseLogger.h"

namespace fm::log {

class StdLogger : public BaseLogger {
 public:
  // StdLogger的主要工作就是向stdout输出日志行

  ~StdLogger() override = default;
  void add(LogLine &&log_line) override;

 private:
  std::atomic_flag flag_{};
};

} // namespace fm::log

#endif //LIBFM_UTIL_STDLOGGER_H_
