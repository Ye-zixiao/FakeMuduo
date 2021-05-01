//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef FAKEMUDUO_BASE_BOUNDBLOCKINGQUEUE_H_
#define FAKEMUDUO_BASE_BOUNDBLOCKINGQUEUE_H_

#include "noncoapyable.h"

#include <condition_variable>
#include <mutex>
#include <queue>

namespace fm {

template<typename T>
class BoundBlockingQueue : private noncopyable {
 public:
  explicit BoundBlockingQueue(size_t maxSize)
	  : mutex_(), notEmpty_(), notFull_(), queue_(), maxSize_(maxSize) {}

  void put(const T &x) {
	std::unique_lock<std::mutex> lock(mutex_);
	while (queue_.size() >= maxSize_)
	  notFull_.wait(lock);
	queue_.push(x);
	lock.unlock();
	notEmpty_.notify_one();
  }

  void put(T &&x) {
	std::unique_lock<std::mutex> lock(mutex_);
	while (queue_.size() >= maxSize_)
	  notFull_.wait(lock);
	queue_.push(std::move(x));
	lock.unlock();
	notEmpty_.notify_one();
  }

  T take() {
	std::unique_lock<std::mutex> lock(mutex_);
	while (queue_.empty())
	  notEmpty_.wait(lock);
	T front = queue_.front();
	queue_.pop();
	lock.unlock();
	notFull_.notify_one();
	return front;
  }

  bool empty() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return queue_.empty();
  }

  bool full() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return queue_.size() >= maxSize_;
  }

  size_t size() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return queue_.size();
  }

  size_t maxSize() const {
	std::lock_guard<std::mutex> lock(mutex_);
	return maxSize_;
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable notFull_;
  std::condition_variable notEmpty_;
  std::queue<T> queue_;
  size_t maxSize_;
};

} // namespace fm

#endif //FAKEMUDUO_BASE_BOUNDBLOCKINGQUEUE_H_
