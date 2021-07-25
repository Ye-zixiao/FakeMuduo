//
// Created by Ye-zixiao on 2021/6/6.
//

#ifndef LIBFM_BASE_SYSTEMCLOCK_H_
#define LIBFM_BASE_SYSTEMCLOCK_H_

#include <chrono>
#include <libfm/base/Timestamp.h>

namespace fm::time {

// 系统时钟，主要用来生成当前时间戳
struct SystemClock {
  constexpr static uint64_t kSecondToNanoseconds = 1'000'000'000;
  constexpr static uint32_t kSecondToMicroseconds = 1'000'000;

  using duration = std::chrono::microseconds;

  static Timestamp now() noexcept {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch());
  }

  static Timestamp zero() noexcept {
    return duration::zero();
  }

  static time_t to_time_t(const Timestamp &time) {
    return std::chrono::duration_cast<std::chrono::seconds>(
        time.timeSinceEpoch()).count();
  }

  static Timestamp from_time_t(time_t t) {
    return std::chrono::seconds(t);
  }

  static timeval to_timeval(const Timestamp &time) {
    return timeval{
        std::chrono::duration_cast<std::chrono::seconds>(time.timeSinceEpoch()).count(),
        time.timeSinceEpoch().count() % kSecondToMicroseconds
    };
  }

  static Timestamp from_timeval(timeval tv) {
    return duration(tv.tv_sec * kSecondToMicroseconds + tv.tv_usec);
  }

  static timespec to_timespec(const Timestamp &time) {
    return timespec{
        std::chrono::duration_cast<std::chrono::seconds>(time.timeSinceEpoch()).count(),
        time.timeSinceEpoch().count() % kSecondToMicroseconds * 1000
    };
  }

  static Timestamp from_timespec(timespec ts) {
    return duration(ts.tv_sec * kSecondToMicroseconds + ts.tv_nsec / 1000);
  }
};

} // namespace fm::time

#endif //LIBFM_BASE_SYSTEMCLOCK_H_
