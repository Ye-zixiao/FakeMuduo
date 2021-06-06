//
// Created by Ye-zixiao on 2021/5/5.
//

#include "libfm/net/Timer.h"

#include "libfm/base/SystemClock.h"

using namespace fm;
using namespace fm::net;

std::atomic_int64_t Timer::numsTimerCreated_ = 0;

void Timer::restart(time::Timestamp when) {
  if (repeat_) expiration_ = when + interval_;
  else expiration_ = time::SystemClock::zero();
}