//
// Created by Ye-zixiao on 2021/8/7.
//

#ifndef LIBFM_INCLUDE_LIBFM_FMLOG_SYSTEMCLOCK_H_
#define LIBFM_INCLUDE_LIBFM_FMLOG_SYSTEMCLOCK_H_

#include <chrono>
#include "libfm/fmutil/TimeStamp.h"

namespace fm::time {

struct SystemClock {
  constexpr static uint64_t kSecondToNanoseconds = 1000 * 1000 * 1000;
  constexpr static uint32_t kSecondToMicroseconds = 1000 * 1000;

  using DefaultDeruation = std::chrono::microseconds;

  static TimeStamp now() noexcept {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch());
  }

  static TimeStamp zero() noexcept {
    return DefaultDeruation::zero();
  }

  static time_t to_time_t(const TimeStamp &time) {
    return std::chrono::duration_cast<std::chrono::seconds>(
        time.timeSinceEpoch()).count();
  }

  static TimeStamp from_time_t(time_t t) {
    return std::chrono::seconds(t);
  }

  static timeval to_timeval(const TimeStamp &time) {
    return timeval{
      std::chrono::duration_cast<std::chrono::seconds>(time.timeSinceEpoch()).count(),
      time.timeSinceEpoch().count() % kSecondToMicroseconds
    };
  }

  static TimeStamp from_timeval(timeval tv) {
    return DefaultDeruation(tv.tv_sec * kSecondToMicroseconds + tv.tv_usec);
  }

  static timespec to_timespec(const TimeStamp &time) {
    return timespec{
      std::chrono::duration_cast<std::chrono::seconds>(time.timeSinceEpoch()).count(),
      time.timeSinceEpoch().count() % kSecondToMicroseconds * 1000
    };
  }

  static TimeStamp from_timespec(timespec ts) {
    return DefaultDeruation(ts.tv_sec * kSecondToMicroseconds + ts.tv_nsec / 1000);
  }
};

} // namespace fm::time

#endif //LIBFM_INCLUDE_LIBFM_FMLOG_SYSTEMCLOCK_H_
