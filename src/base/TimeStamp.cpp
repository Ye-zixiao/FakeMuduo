//
// Created by Ye-zixiao on 2021/4/7.
//

#include "TimeStamp.h"

#include <sys/time.h>

using namespace fm;

TimeStamp::TimeStamp(const timeval &tv) :
    usSinceEpoch_(static_cast<int64_t>(tv.tv_sec * kUsPerSecond + tv.tv_usec)) {}

TimeStamp &TimeStamp::operator=(const timeval &tv) {
  usSinceEpoch_ = static_cast<int64_t>(tv.tv_sec * kUsPerSecond + tv.tv_usec);
  return *this;
}

TimeStamp::TimeStamp(const timespec &ts) :
    usSinceEpoch_(static_cast<int64_t>(ts.tv_sec * kUsPerSecond
        + ts.tv_nsec / kNsPerUSecond)) {}

TimeStamp &TimeStamp::operator=(const timespec &ts) {
  usSinceEpoch_ = static_cast<int64_t>(
      ts.tv_sec * kUsPerSecond + ts.tv_nsec / kNsPerUSecond);
  return *this;
}

/***
 * 返回格式化的日期时间字符串
 * @param showUSeconds 是否显示毫秒
 * @return             格式化日期事件字符串
 */
std::string TimeStamp::toFormattedString(bool showUSeconds) const {
  auto seconds = static_cast<time_t>(usSinceEpoch_ / kUsPerSecond);
  struct tm tm_time{};
  char buf[64]{};

//  gmtime_r(&seconds, &tm_time);
// 不返回UTC时间！
  localtime_r(&seconds, &tm_time);

  if (showUSeconds) {
    int useconds = static_cast<int>(usSinceEpoch_ % kUsPerSecond);
    snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, useconds);
  } else {
    snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
             tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
             tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }
  return buf;
}

std::string TimeStamp::toString() const {
  char buf[32]{};
  int64_t seconds = usSinceEpoch_ / kUsPerSecond;
  int64_t useconds = usSinceEpoch_ % kUsPerSecond;
  snprintf(buf, sizeof(buf), "%ld.%06ld", seconds, useconds);
  return buf;
}

/***
 * 返回记录当前时刻的时间戳TimeStamp对象
 * @return 返回当前时间戳
 */
TimeStamp TimeStamp::now() {
  struct timeval tv{};
  gettimeofday(&tv, nullptr);
  return TimeStamp(tv);
}