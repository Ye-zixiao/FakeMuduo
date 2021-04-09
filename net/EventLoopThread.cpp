//
// Created by Ye-zixiao on 2021/4/9.
//

#include "EventLoopThread.h"

#include <cassert>
#include <unistd.h>

#include "../base/Logging.h"
#include "EventLoop.h"

using namespace fm;
using namespace fm::net;

EventLoopThread::EventLoopThread(const std::string &name)
	: loop_(nullptr),
	  exiting_(false),
	  thread_(),
	  mutex_(),
	  cond_() {}

EventLoopThread::~EventLoopThread() {
  exiting_ = true;
  if (loop_) {
	loop_->quit();
	thread_->join();
  }
}

EventLoop *EventLoopThread::startLoop() {
  assert(!thread_);
  thread_ = std::make_unique<std::thread>(std::bind(&EventLoopThread::threadFunc, this));

  EventLoop *loop = nullptr;
  {
	std::unique_lock<std::mutex> lock(mutex_);
	while (!loop_)
	  cond_.wait(lock);
	loop = loop_;
  }
  return loop;
}

void EventLoopThread::threadFunc() {
  LOG_TRACE<<"threadFunc(): this thread is "<< gettid();
  EventLoop loop;
  LOG_TRACE<<"threadFunc(): loop's thread is "<<loop.threadId();
  {
	std::lock_guard<std::mutex> lock(mutex_);
	loop_ = &loop;
	cond_.notify_all();
  }

  loop.loop();
  std::lock_guard<std::mutex> lock(mutex_);
  loop_ = nullptr;
}