//
// Created by Ye-zixiao on 2021/8/7.
//

#ifndef LIBFM_INCLUDE_LIBFM_FMLOG_TIMESTAMP_H_
#define LIBFM_INCLUDE_LIBFM_FMLOG_TIMESTAMP_H_

#include <string>
#include <chrono>

namespace fm::time {

using namespace std::chrono_literals;

class TimeStamp {
 public:
  // 默认的时间长度内部使用一个int64进行记录
  using DefaultDuration = std::chrono::microseconds;

  template<typename Rep, typename Period>
  TimeStamp(std::chrono::duration<Rep, Period> time)
  :time_since_epoch_(time) {}

  TimeStamp(DefaultDuration time) : time_since_epoch_(time) {}
  TimeStamp() : time_since_epoch_(DefaultDuration::zero()) {}

  std::string toFormattedString(const char *fmt) const;
  std::string toString(bool microsecond = false) const;

  DefaultDuration timeSinceEpoch() const { return time_since_epoch_; }
  bool isZero() const { return time_since_epoch_ == DefaultDuration::zero(); }

  TimeStamp &operator+=(const DefaultDuration &rhs);
  TimeStamp &operator-=(const DefaultDuration &rhs);

 private:
  DefaultDuration time_since_epoch_;
};

inline TimeStamp &TimeStamp::operator+=(const DefaultDuration &rhs) {
  this->time_since_epoch_ += rhs;
  return *this;
}

inline TimeStamp &TimeStamp::operator-=(const DefaultDuration &rhs) {
  this->time_since_epoch_ -= rhs;
  return *this;
}

inline TimeStamp::DefaultDuration operator-(const TimeStamp &lhs,
    const TimeStamp &rhs) {
  return lhs.timeSinceEpoch() - rhs.timeSinceEpoch();
}

template<typename Rep, typename Period>
inline TimeStamp operator+(const TimeStamp &lhs,
    const std::chrono::duration<Rep, Period> &rhs) {
  return lhs.timeSinceEpoch() + rhs;
}

template<typename Rep, typename Period>
inline TimeStamp operator+(const std::chrono::duration<Rep, Period> &lhs,
    const TimeStamp &rhs) {
  return lhs + rhs.timeSinceEpoch();
}

template<typename Rep, typename Period>
inline TimeStamp operator-(const TimeStamp &lhs,
    const std::chrono::duration<Rep, Period> &rhs) {
  return lhs.timeSinceEpoch() - rhs;
}

inline bool operator<(const TimeStamp &lhs, const TimeStamp &rhs) {
  return lhs.timeSinceEpoch() < rhs.timeSinceEpoch();
}

inline bool operator>(const TimeStamp &lhs, const TimeStamp &rhs) {
  return lhs.timeSinceEpoch() > rhs.timeSinceEpoch();
}

inline bool operator==(const TimeStamp &lhs, const TimeStamp &rhs) {
  return lhs.timeSinceEpoch() == rhs.timeSinceEpoch();
}

inline bool operator!=(const TimeStamp &lhs, const TimeStamp &rhs) {
  return lhs.timeSinceEpoch() != rhs.timeSinceEpoch();
}

} // namespace fm::time

namespace std {

template<>
struct hash<fm::time::TimeStamp> {
  using argument_type = fm::time::TimeStamp;
  using result_type = size_t;
  result_type operator()(const argument_type &arg) const {
    return hash<int64_t>{}(arg.timeSinceEpoch().count());
  }
};

} // namespace std

#endif //LIBFM_INCLUDE_LIBFM_FMLOG_TIMESTAMP_H_
