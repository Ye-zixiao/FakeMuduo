//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef FAKEMUDUO_BASE_COUNTDOWNLATCH_H_
#define FAKEMUDUO_BASE_COUNTDOWNLATCH_H_

#include "noncopyable.h"

#include <condition_variable>
#include <mutex>

namespace fm {

class CountDownLatch : private noncopyable {
 public:
  explicit CountDownLatch(size_t count)
	  : mutex_(), cond_(), counter_(count) {}

  void wait() {
	std::unique_lock<std::mutex> lock(mutex_);
	while (counter_ == 0)
	  cond_.wait(lock);
  }

  void countDown() {
	std::lock_guard<std::mutex> lock(mutex_);
	if (--counter_ == 0)
	  cond_.notify_all();
  }

  int getCount() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return counter_;
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable cond_;
  size_t counter_;
};

} // namespace fm

#endif //FAKEMUDUO_BASE_COUNTDOWNLATCH_H_
