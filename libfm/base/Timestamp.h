//
// Created by Ye-zixiao on 2021/6/6.
//

#ifndef FAKEMUDUO_LIBFM_BASE_TIMESTAMP1_H_
#define FAKEMUDUO_LIBFM_BASE_TIMESTAMP1_H_

#include <chrono>
#include <string>

#include <libfm/base/copyable.h>

namespace fm::time {

using namespace std::chrono_literals;

class Timestamp : public copyable {
 public:
  using duration = std::chrono::microseconds;

  template<typename Rep, typename Period>
  Timestamp(std::chrono::duration<Rep, Period> time)
      :timeSinceEpoch_(time) {}
  Timestamp(duration time)
      : timeSinceEpoch_(time) {}
  Timestamp()
      : timeSinceEpoch_(duration::zero()) {}

  std::string toFormattedString(const char *fmt) const;
  std::string toString(bool microsecond = false) const;

  duration timeSinceEpoch() const { return timeSinceEpoch_; }

  bool isZero() const { return timeSinceEpoch_ == duration::zero(); }

  Timestamp &operator+=(const duration &rhs) {
    timeSinceEpoch_ += rhs;
    return *this;
  }

  Timestamp &operator-=(const duration &rhs) {
    timeSinceEpoch_ -= rhs;
    return *this;
  }

 private:
  duration timeSinceEpoch_;
};

template<typename Rep, typename Period>
inline Timestamp operator+(const Timestamp &lhs,
                           const std::chrono::duration<Rep, Period> &rhs) {
  return lhs.timeSinceEpoch() + rhs;
}

template<typename Rep, typename Period>
inline Timestamp operator+(const std::chrono::duration<Rep, Period> &lhs,
                           const Timestamp &rhs) {
  return lhs + rhs.timeSinceEpoch();
}

template<typename Rep, typename Period>
inline Timestamp operator-(const Timestamp &lhs,
                           const std::chrono::duration<Rep, Period> &rhs) {
  return lhs.timeSinceEpoch() - rhs;
}

inline time::Timestamp::duration operator-(const time::Timestamp &lhs,
                                    const time::Timestamp &rhs) {
  return lhs.timeSinceEpoch() - rhs.timeSinceEpoch();
}

inline bool operator<(const Timestamp &lhs, const Timestamp &rhs) {
  return lhs.timeSinceEpoch() < rhs.timeSinceEpoch();
}

inline bool operator>(const Timestamp &lhs, const Timestamp &rhs) {
  return lhs.timeSinceEpoch() > rhs.timeSinceEpoch();
}

inline bool operator==(const Timestamp &lhs, const Timestamp &rhs) {
  return lhs.timeSinceEpoch() == rhs.timeSinceEpoch();
}

inline bool operator!=(const Timestamp &lhs, const Timestamp &rhs) {
  return !(lhs == rhs);
}

} // namespace fm::time

namespace std {
template<>
struct hash<fm::time::Timestamp> {
  using argument_type = fm::time::Timestamp;
  using result_type = size_t;
  result_type operator()(const argument_type &rhs) const {
    return hash<int64_t>{}(rhs.timeSinceEpoch().count());
  }
};
} // namespace std;

#endif //FAKEMUDUO_LIBFM_BASE_TIMESTAMP1_H_
