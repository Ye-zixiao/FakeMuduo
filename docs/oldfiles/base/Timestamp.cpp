//
// Created by Ye-zixiao on 2021/6/6.
//

#include "Timestamp.h"
#include <ctime>

using namespace fm::time;

std::string Timestamp::toFormattedString(const char *fmt) const {
  time_t sec = std::chrono::duration_cast<std::chrono::seconds>(
      time_since_epoch_).count();
  struct tm tm_time{};
  char buf[64]{};

  localtime_r(&sec, &tm_time);
  strftime(buf, 64, fmt, &tm_time);
  return buf;
}

std::string Timestamp::toString(bool microsecond) const {
  if (microsecond) {
    constexpr int32_t kSecondsToMicroseconds = 1'000'000;
    uint32_t rest = time_since_epoch_.count() % kSecondsToMicroseconds;
    char buf[12]{};

    snprintf(buf, 12, ".%06d", rest);
    return toFormattedString("%F %T").append(buf);
  }
  return toFormattedString("%F %T");
}