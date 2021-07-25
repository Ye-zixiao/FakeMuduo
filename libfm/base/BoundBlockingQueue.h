//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef LIBFM_BASE_BOUNDBLOCKINGQUEUE_H_
#define LIBFM_BASE_BOUNDBLOCKINGQUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>
#include "libfm/base/NonCopyable.h"

namespace fm {

template<typename T>
class BoundBlockingQueue : private NonCopyable {
 public:
  explicit BoundBlockingQueue(size_t maxSize)
      : mutex_(), not_empty_(), not_full_(), queue_(), max_size_(maxSize) {}

  void put(const T &x) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_full_.wait(lock, [this] { queue_.size() >= max_size_; });
    queue_.push(x);
    lock.unlock();
    not_empty_.notify_one();
  }

  void put(T &&x) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_full_.wait(lock, [this] { return queue_.size() >= max_size_; });
    queue_.push(std::move(x));
    lock.unlock();
    not_empty_.notify_one();
  }

  T take() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty())
      not_empty_.wait(lock);
    T front = queue_.front();
    queue_.pop();
    lock.unlock();
    not_full_.notify_one();
    return front;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  bool full() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size() >= max_size_;
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

  size_t maxSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return max_size_;
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable not_full_;
  std::condition_variable not_empty_;
  std::queue<T> queue_;
  size_t max_size_;
};

} // namespace fm

#endif //LIBFM_BASE_BOUNDBLOCKINGQUEUE_H_
