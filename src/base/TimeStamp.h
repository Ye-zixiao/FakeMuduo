//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef FAKEMUDUO_BASE_TIMESTAMP_H_
#define FAKEMUDUO_BASE_TIMESTAMP_H_

#include "copyable.h"

#include <string>

namespace fm {

class TimeStamp : private copyable {
 public:
  TimeStamp() : usSinceEpoch_(0) {}

  explicit TimeStamp(int64_t usSinceEpoch)
      : usSinceEpoch_(usSinceEpoch) {}

  TimeStamp(const TimeStamp &rhs) = default;
  TimeStamp &operator=(const TimeStamp &rhs) = default;

  explicit TimeStamp(const timeval &tv);
  TimeStamp &operator=(const timeval &tv);

  explicit TimeStamp(const timespec &ts);
  TimeStamp &operator=(const timespec &ts);

  std::string toFormattedString(bool showUSeconds = true) const;
  std::string toString() const;

  int64_t usSinceEpoch() const { return usSinceEpoch_; }
  time_t secondsSinceEpoch() const { return static_cast<time_t>(usSinceEpoch_ / kUsPerSecond); }

  bool isValid() const { return usSinceEpoch_ > 0; }

  void swap(TimeStamp &that) { std::swap(usSinceEpoch_, that.usSinceEpoch_); }

 public:
  static constexpr int kMsPerSecond = 1000;
  static constexpr int kUsPerSecond = 1000 * 1000;
  static constexpr int kNsPerSecond = 1000 * 1000 * 1000;
  static constexpr int kUsPerMSecond = 1000;
  static constexpr int kNsPerMSecond = 1000 * 1000;
  static constexpr int kNsPerUSecond = 1000;

  static TimeStamp now();
  static TimeStamp invalid() { return TimeStamp(); }

 private:
  // 注意，微秒的英文为microsecond，毫秒的英文为millisecond，但
  // 为了符合个人错误的习惯，还是将微秒写成us而不是microSeconds！
  int64_t usSinceEpoch_;
};

inline bool operator<(const TimeStamp &lhs, const TimeStamp &rhs) {
  return lhs.usSinceEpoch() < rhs.usSinceEpoch();
}

inline bool operator>(const TimeStamp &lhs, const TimeStamp &rhs) {
  return lhs.usSinceEpoch() > rhs.usSinceEpoch();
}

inline bool operator==(const TimeStamp &lhs, const TimeStamp &rhs) {
  return lhs.usSinceEpoch() == rhs.usSinceEpoch();
}

// 返回相差多少秒
inline double timeDiff(const TimeStamp &lhs, const TimeStamp &rhs) {
  uint64_t diff = lhs.usSinceEpoch() - rhs.usSinceEpoch();
  return static_cast<double>(diff) / TimeStamp::kUsPerSecond;
}

inline TimeStamp timeAdd(TimeStamp time, double seconds) {
  auto delta = static_cast<int64_t>(seconds * TimeStamp::kUsPerSecond);
  return TimeStamp(time.usSinceEpoch() + delta);
}

} // namespace fm

#endif //FAKEMUDUO_BASE_TIMESTAMP_H_
