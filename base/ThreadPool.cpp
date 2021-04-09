//
// Created by Ye-zixiao on 2021/4/7.
//

#include "ThreadPool.h"

#include <unistd.h>

using namespace fm;

ThreadPool::ThreadPool(std::string str, size_t maxSize)
	: name_(str), mutex_(), notEmpty_(), notFull_(), queue_(),
	  threads_(), running_(false), maxSize_(maxSize) {}

ThreadPool::~ThreadPool() {
  if (running_) stop();
}

void ThreadPool::start(int numThreads) {
  running_ = true;
  threads_.reserve(numThreads);
  for (int i = 0; i < numThreads; ++i)
	threads_.emplace_back(std::make_unique<std::thread>(
		std::bind(&ThreadPool::runInThread, this)));
}

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
	task();
  } else {
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
	notFull_.notify_one();
  }
  return task;
}

void ThreadPool::runInThread() {
  int counter=0;
  // 如果running_被设为false但是主线程仍然向任务队列中
  // 发送新的任务怎么办？所以则必须要求主线程提前退出事件循环
  while (running_ || !isEmptyQueue()) {
	Task task(take());
	if (task) {
	  task();
	  ++counter;
	}
  }
  printf("thread %d counts %d\n",gettid(),counter);
}

bool ThreadPool::isEmptyQueue() {
  std::lock_guard<std::mutex> lock(mutex_);
  return queue_.empty();
}
