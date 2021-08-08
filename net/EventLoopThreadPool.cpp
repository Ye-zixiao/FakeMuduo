//
// Created by Ye-zixiao on 2021/4/9.
//

#include "net/EventLoopThreadPool.h"
#include <cassert>
#include "libfm/fmutil/Log.h"
#include "libfm/fmnet/EventLoopThread.h"
#include "libfm/fmnet/EventLoop.h"

using namespace fm;
using namespace fm::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop,
                                         std::string name)
    : baseLoop_(baseLoop),
      name_(std::move(name)),
      start_(false),
      numThreads_(0),
      next_(0),
      threads_(),
      loops_() {}

void EventLoopThreadPool::start() {
  assert(!start_);
  baseLoop_->assertInLoopThread();

  start_ = true;
  for (int i = 0; i < numThreads_; ++i) {
    auto *t = new EventLoopThread();
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->startLoop());
  }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
  baseLoop_->assertInLoopThread();
  assert(start_);
  EventLoop *loop = baseLoop_;

  if (!loops_.empty()) {
    loop = loops_[next_++];
    next_ %= loops_.size();
  }
  return loop;
}

EventLoop *EventLoopThreadPool::getLoopForHash(size_t hashCode) {
  baseLoop_->assertInLoopThread();
  EventLoop *loop = baseLoop_;

  if (!loops_.empty())
    loop = loops_[hashCode % loops_.size()];
  return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
  baseLoop_->assertInLoopThread();
  assert(start_);
  return loops_.empty() ? std::vector<EventLoop *>{baseLoop_} : loops_;
}