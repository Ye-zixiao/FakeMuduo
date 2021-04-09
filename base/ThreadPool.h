//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef FAKEMUDUO_BASE_THREADPOOL_H_
#define FAKEMUDUO_BASE_THREADPOOL_H_

#include "noncoapyable.h"

#include <condition_variable>
#include <functional>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <queue>

namespace fm {

class ThreadPool : public noncopyable {
 public:
  using Task = std::function<void()>;
  using ThreadPtr = std::unique_ptr<std::thread>;

  ThreadPool(std::string str, size_t maxSize);

  ~ThreadPool();

  void start(int numThreads);

  void stop();

  void run(Task task);

  const std::string &name() const { return name_; }

 private:
  void runInThread();
  Task take();
  bool isEmptyQueue();

 private:
  std::string name_;
  std::mutex mutex_;
  std::condition_variable notEmpty_;
  std::condition_variable notFull_;
  std::queue<Task> queue_;
  std::vector<ThreadPtr> threads_;
  // can be replaced by atomic_bool?
  bool running_;
  size_t maxSize_;
};

} // namespace fm

#endif //FAKEMUDUO_BASE_THREADPOOL_H_
