//
// Created by Ye-zixiao on 2021/5/5.
//

#ifndef LIBFM_NET_TIMER_H_
#define LIBFM_NET_TIMER_H_

#include <functional>
#include <atomic>
#include "libfm/fmutil/TimeStamp.h"
#include "libfm/fmnet/Callback.h"

namespace fm::net {

class Timer {
 public:
  using duration = time::TimeStamp::DefaultDuration;

  Timer(TimerCallback cb, time::TimeStamp when, duration interval)
      : callback_(std::move(cb)),
        expiration_(when),
        interval_(interval),
        repeat_(interval > duration::zero()),
        sequence_(numsTimerCreated_++) {}

  Timer(const Timer &) = delete;
  Timer &operator=(const Timer &) = delete;

  void run() const { callback_(); }
  void restart(time::TimeStamp when);

  time::TimeStamp expiration() const { return expiration_; }
  bool isRepeated() const { return repeat_; }
  int64_t sequence() const { return sequence_; }

  static int64_t numsTimerCreate() { return numsTimerCreated_.load(); }

 private:
  const TimerCallback callback_;                    // 用户回调函数
  time::TimeStamp expiration_;                      // 过期时间
  const duration interval_;                         // 定时器周期
  const bool repeat_;                               // 是否周期定时
  const int64_t sequence_;                          // 定时器序号

  static std::atomic_int64_t numsTimerCreated_;
};

} // namespace fm::net

#endif //LIBFM_NET_TIMER_H_
