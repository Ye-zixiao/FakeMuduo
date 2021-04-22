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
  TimeStamp() : microSecondsSinceEpoch_(0) {}

  explicit TimeStamp(int64_t microSecondsSinceEpoch)
	  : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}

  void swap(TimeStamp &that) {
	std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
  }

  std::string toString() const;
  std::string toFormattedString(bool showMicroseconds = true) const;

  bool isValid() const { return microSecondsSinceEpoch_ > 0; }

  int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
  time_t secondsSinceEpoch() const {
	return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
  }

 public:
  static TimeStamp now();
  static TimeStamp invalid() { return TimeStamp(); }
  static constexpr int kMicroSecondsPerSecond = 1000 * 1000;

 private:
  uint64_t microSecondsSinceEpoch_;
};

inline bool operator<(const TimeStamp &lhs, const TimeStamp &rhs) {
  return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator>(const TimeStamp &lhs, const TimeStamp &rhs) {
  return lhs.microSecondsSinceEpoch() > rhs.microSecondsSinceEpoch();
}

inline bool operator==(const TimeStamp &lhs, const TimeStamp &rhs) {
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

// timeDiff的另一种提供方式
inline double operator-(const TimeStamp &lhs, const TimeStamp &rhs) {
  int64_t diff = lhs.microSecondsSinceEpoch() - rhs.microSecondsSinceEpoch();
  return static_cast<double>(diff) / TimeStamp::kMicroSecondsPerSecond;
}

// 返回相差多少秒
inline double timeDiff(const TimeStamp &lhs, const TimeStamp &rhs) {
  int64_t diff = lhs.microSecondsSinceEpoch() - rhs.microSecondsSinceEpoch();
  return static_cast<double>(diff) / TimeStamp::kMicroSecondsPerSecond;
}

} // namespace fm

#endif //FAKEMUDUO_BASE_TIMESTAMP_H_
