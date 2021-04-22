//
// Created by Ye-zixiao on 2021/4/7.
//

#include "TimeStamp.h"

#include <sys/time.h>

using namespace fm;

/***
 * 返回格式化的日期时间字符串
 * @param showMicroseconds
 * @return
 */
std::string TimeStamp::toFormattedString(bool showMicroseconds) const {
  auto seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
  struct tm tm_time{};
  char buf[64]{};

//  gmtime_r(&seconds, &tm_time);
// 不返回UTC时间！
  localtime_r(&seconds, &tm_time);

  if (showMicroseconds) {
	int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
	snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
			 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
  } else {
	snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
			 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
			 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
  }
  return buf;
}

std::string TimeStamp::toString() const {
  char buf[32]{};
  int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
  int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;
  snprintf(buf, sizeof(buf), "%ld.%06ld", seconds, microseconds);
  return buf;
}

/***
 * 返回记录当前时刻的时间戳TimeStamp对象
 * @return
 */
TimeStamp TimeStamp::now() {
  struct timeval tv{};
  gettimeofday(&tv, nullptr);
  int64_t seconds = tv.tv_sec;
  return TimeStamp(seconds * kMicroSecondsPerSecond + tv.tv_usec);
}