//
// Created by Ye-zixiao on 2021/4/9.
//

#include "libfm/net/EventLoopThreadPool.h"

#include <cassert>

#include "libfm/base/Logging.h"
#include "libfm/net/EventLoopThread.h"
#include "libfm/net/EventLoop.h"

using namespace fm;
using namespace fm::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop,
                                         const std::string &name)
    : baseLoop_(baseLoop),
      name_(name),
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
    char buf[name_.size() + 32];
    snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
    auto *t = new EventLoopThread(buf);
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
    next_ = next_ % loops_.size();
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