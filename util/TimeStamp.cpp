//
// Created by Ye-zixiao on 2021/8/7.
//

#include "libfm/fmutil/TimeStamp.h"
#include <ctime>

using namespace fm::time;

std::string TimeStamp::toFormattedString(const char *fmt) const {
  time_t sec = std::chrono::duration_cast<std::chrono::seconds>(
      this->time_since_epoch_).count();
  struct tm tm_time{};
  char buf[64]{};

  localtime_r(&sec, &tm_time);
  strftime(buf, 64, fmt, &tm_time);
  return buf;
}

std::string TimeStamp::toString(bool microsecond) const {
  if (microsecond) {
    constexpr uint32_t kSecondsToMicroseconds = 1000 * 1000;
    uint32_t rest = static_cast<uint32_t>(time_since_epoch_.count())
        % kSecondsToMicroseconds;
    char buf[12]{};

    snprintf(buf, sizeof(buf), ".%06d", rest);
    return toFormattedString("%F %T").append(buf);
  }
  return toFormattedString("%F %T");
}