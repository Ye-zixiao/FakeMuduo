//
// Created by Ye-zixiao on 2021/5/5.
//

#include "Timer.h"

using namespace fm;
using namespace fm::net;

std::atomic_int64_t Timer::numsTimerCreated_ = 0;

void Timer::restart(TimeStamp when) {
  if (repeat_) expiration_ = timeAdd(when, interval_);
  else expiration_ = TimeStamp::invalid();
}