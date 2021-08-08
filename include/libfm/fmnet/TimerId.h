//
// Created by Ye-zixiao on 2021/8/8.
//

#ifndef OLD_LIBFM_INCLUDE_LIBFM_FMNET_TIMERID_H_
#define OLD_LIBFM_INCLUDE_LIBFM_FMNET_TIMERID_H_

#include <functional>

namespace fm::net {

class Timer;

// 定时器标识
class TimerId {
 public:
  friend class TimerQueue;
  friend struct std::hash<TimerId>;

  TimerId() : timer_(nullptr), sequence_(0) {}
  TimerId(Timer *timer, int64_t seq) : timer_(timer), sequence_(seq) {}
  TimerId(const TimerId &) = default;
  TimerId &operator=(const TimerId &) = default;

  Timer *timer() const { return timer_; }
  int64_t sequence() const { return sequence_; }

  inline friend bool operator<(const TimerId &lhs, const TimerId &rhs) {
    return lhs.timer_ < rhs.timer_ && lhs.sequence_ < rhs.sequence_;
  }

  inline friend bool operator==(const TimerId &lhs, const TimerId &rhs) {
    return lhs.timer_ == rhs.timer_ && lhs.sequence_ == rhs.sequence_;
  }

 private:
  Timer *timer_;
  int64_t sequence_;
};

} // namespace fm::net

namespace std {
template<>
struct hash<fm::net::TimerId> {
  using argument_type = fm::net::TimerId;
  using result_type = size_t;

  result_type operator()(const argument_type &timerId) const {
    const result_type h1(std::hash<fm::net::Timer *>{}(timerId.timer_));
    const result_type h2(std::hash<int64_t>{}(timerId.sequence_));
    return h1 ^ (h2 << 1);
  }
};
} // namespace std

#endif //OLD_LIBFM_INCLUDE_LIBFM_FMNET_TIMERID_H_
