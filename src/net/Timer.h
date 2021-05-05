//
// Created by Ye-zixiao on 2021/5/5.
//

#ifndef FAKEMUDUO_SRC_NET_TIMER_H_
#define FAKEMUDUO_SRC_NET_TIMER_H_

#include <atomic>

#include "../base/noncoapyable.h"
#include "../base/copyable.h"
#include "../base/TimeStamp.h"
#include "Callback.h"

namespace fm {
namespace net {

class Timer : private noncopyable {
 public:
  Timer(TimerCallback cb, TimeStamp when, double interval)
      : callback_(std::move(cb)),
        expiration_(when),
        interval_(interval),
        repeat_(interval > 0),
        sequence_(numsTimerCreated_++) {}

  void run() const { callback_(); }
  void restart(TimeStamp when);

  TimeStamp expiration() const { return expiration_; }
  bool isRepeated() const { return repeat_; }
  int64_t sequence() const { return sequence_; }

  static int64_t numsTimerCreate() { return numsTimerCreated_.load(); }

 private:
  const TimerCallback callback_; // 用户回调函数
  TimeStamp expiration_;         // 过期时间
  const double interval_;        // 定时器周期
  const bool repeat_;            // 是否周期定时
  const int64_t sequence_;       // 定时器序号

  static std::atomic_int64_t numsTimerCreated_;
};

// 定时器标识
class TimerId : public copyable {
 public:
  friend class TimerQueue;

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

} // namespace net
} // namespace fm

#endif //FAKEMUDUO_SRC_NET_TIMER_H_
