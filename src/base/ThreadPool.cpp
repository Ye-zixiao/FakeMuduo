//
// Created by Ye-zixiao on 2021/4/7.
//

#include "ThreadPool.h"

#include <unistd.h>

#include <memory>

using namespace fm;

ThreadPool::ThreadPool(std::string str, size_t maxSize)
    : name_(std::move(str)),
      mutex_(),
      notEmpty_(),
      notFull_(),
      queue_(),
      threads_(),
      running_(false),
      maxSize_(maxSize) {}

ThreadPool::~ThreadPool() {
  if (running_) stop();
}

void ThreadPool::setThreadNum(int numThreads) {
  threads_.resize(numThreads);
}

void ThreadPool::start() {
  running_ = true;
  for(auto&up:threads_)
    // 创建并启动工作线程，然后将其独一指针加入到线程容器中
    up = std::make_unique<std::thread>(&ThreadPool::runInThread,this);
}

//void ThreadPool::start(int numThreads) {
//  running_ = true;
//  threads_.reserve(numThreads);
//  for (int i = 0; i < numThreads; ++i)
//    // 创建/启动线程，并以移动的方式加入到线程容器中
//    threads_.emplace_back(std::make_unique<std::thread>(
//        std::bind(&ThreadPool::runInThread, this)));
//}

void ThreadPool::stop() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
    notEmpty_.notify_all();
  }
  for (auto &thr:threads_)
    thr->join();
}

void ThreadPool::run(Task task) {
  if (threads_.empty()) {
    // 若线程池中并没有工作线程，则这个任务直接在当前线程中完成
    task();
  } else {
    // 否则将会调用的可调用对象加入到任务队列中
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.size() >= maxSize_)
      notFull_.wait(lock);
    queue_.push(std::move(task));
    lock.unlock();
    notEmpty_.notify_one();
  }
}

ThreadPool::Task ThreadPool::take() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (queue_.empty() && running_)
    notEmpty_.wait(lock);
  Task task;
  if (!queue_.empty()) {
    task = queue_.front();
    queue_.pop();
  }
  lock.unlock();
  notFull_.notify_one();
  return task;
}

void ThreadPool::runInThread() {
//#define DEBUG
#ifdef DEBUG
  int counter = 0;
#endif
  // 如果running_被设为false但是主线程仍然向任务队列中
  // 发送新的任务怎么办？所以则必须要求主线程提前退出事件循环
  while (running_ || !isEmptyQueue()) {
    Task task(take());
    if (task) {
      task();
#ifdef DEBUG
      ++counter;
      printf("thread %d counts %d\n", gettid(), counter);
#endif
    }
  }
#ifdef DEBUG
  printf("thread %d counts %d\n", gettid(), counter);
#endif
}

bool ThreadPool::isEmptyQueue() {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.empty();
}
