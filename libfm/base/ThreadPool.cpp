//
// Created by Ye-zixiao on 2021/4/7.
//

#include "libfm/base/ThreadPool.h"
#include <unistd.h>
#include <memory>

using namespace fm;

ThreadPool::ThreadPool(std::string str, size_t maxSize)
    : name_(std::move(str)),
      mutex_(),
      not_empty_(),
      not_full_(),
      queue_(),
      threads_(),
      running_(false),
      max_size_(maxSize) {}

ThreadPool::~ThreadPool() {
  if (running_) stop();
}

void ThreadPool::setThreadNum(int numThreads) {
  threads_.resize(numThreads);
}

void ThreadPool::start() {
  running_ = true;
  for (auto &up:threads_)
    up = std::make_unique<std::thread>(&ThreadPool::runInThread, this);
}

void ThreadPool::stop() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
    not_empty_.notify_all();
  }
  for (auto &thr:threads_)
    thr->join();
}

void ThreadPool::submit(Task task) {
  if (threads_.empty()) {
    // 若线程池中并没有工作线程，则这个任务直接在当前线程中完成
    task();
  } else {
    // 否则将会调用的可调用对象加入到任务队列中
    std::unique_lock<std::mutex> lock(mutex_);
    not_full_.wait(lock, [this] { return queue_.size() >= max_size_; });
    queue_.push(std::move(task));
    lock.unlock();
    not_empty_.notify_one();
  }
}

ThreadPool::Task ThreadPool::take() {
  std::unique_lock<std::mutex> lock(mutex_);
  not_empty_.wait(lock, [this] { return queue_.empty() && running_; });
  Task task;
  if (!queue_.empty()) {
    task = queue_.front();
    queue_.pop();
  }
  lock.unlock();
  not_full_.notify_one();
  return task;
}

void ThreadPool::runInThread() {
  // 如果running_被设为false但是主线程仍然向任务队列中
  // 发送新的任务怎么办？所以则必须要求主线程提前退出事件循环
  while (running_ || !isEmptyQueue()) {
    Task task(take());
    if (task) task();
  }
}

bool ThreadPool::isEmptyQueue() {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.empty();
}
