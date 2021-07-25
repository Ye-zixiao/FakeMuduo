//
// Created by Ye-zixiao on 2021/5/5.
//

#ifndef LIBFM_NET_TIMER_H_
#define LIBFM_NET_TIMER_H_

#include <functional>
#include <atomic>
#include "libfm/base/NonCopyable.h"
#include "libfm/base/Copyable.h"
#include "libfm/base/Timestamp.h"
#include "libfm/net/Callback.h"

namespace fm::net {

class Timer : private NonCopyable {
 public:
  using duration = time::Timestamp::duration;

  Timer(TimerCallback cb, time::Timestamp when, duration interval)
      : callback_(std::move(cb)),
        expiration_(when),
        interval_(interval),
        repeat_(interval > duration::zero()),
        sequence_(numsTimerCreated_++) {}

  void run() const { callback_(); }
  void restart(time::Timestamp when);

  time::Timestamp expiration() const { return expiration_; }
  bool isRepeated() const { return repeat_; }
  int64_t sequence() const { return sequence_; }

  static int64_t numsTimerCreate() { return numsTimerCreated_.load(); }

 private:
  const TimerCallback callback_;                    // 用户回调函数
  time::Timestamp expiration_;                      // 过期时间
  const duration interval_;                         // 定时器周期
  const bool repeat_;                               // 是否周期定时
  const int64_t sequence_;                          // 定时器序号

  static std::atomic_int64_t numsTimerCreated_;
};

// 定时器标识
class TimerId : public Copyable {
 public:
  friend class TimerQueue;
  friend struct std::hash<TimerId>;

  inline friend bool operator<(const TimerId &lhs, const TimerId &rhs) {
    return lhs.timer_ < rhs.timer_ && lhs.sequence_ < rhs.sequence_;
  }

  inline friend bool operator==(const TimerId &lhs, const TimerId &rhs) {
    return lhs.timer_ == rhs.timer_ && lhs.sequence_ == rhs.sequence_;
  }

  TimerId() :
      timer_(nullptr), sequence_(0) {}

  TimerId(Timer *timer, int64_t seq)
      : timer_(timer), sequence_(seq) {}

  Timer *timer() const { return timer_; }
  int64_t sequence() const { return sequence_; }

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

#endif //LIBFM_NET_TIMER_H_
