//
// Created by Ye-zixiao on 2021/4/7.
//

#include "EventLoop.h"

#include <algorithm>
#include <csignal>
#include <cassert>
#include <sys/eventfd.h>
#include <unistd.h>

#include "../base/Logging.h"
#include "Channel.h"
#include "Poller.h"
#include "SocketsOps.h"

using namespace fm;
using namespace fm::net;

namespace {

thread_local EventLoop *loopInThisThread_ts = nullptr;
constexpr int kPollTimeMs = 10000; // 10s

int createNonblockingEventfdOrDie() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
	LOG_FATAL << "Failed in eventfd";
  return evtfd;
}

struct IgnoreSigPipe {
  IgnoreSigPipe() {
	::signal(SIGPIPE, SIG_IGN);
  }
};

// 程序创建之初就忽略SIGPIPE信号
IgnoreSigPipe initToIgnoreSigPipe;

}

EventLoop::EventLoop()
	: looping_(false),
	  quit_(false),
	  eventHandling_(false),
	  callingPendingFunctors_(false),
	  iteration_(0),
	  threadId_(gettid()),
	  pollReturnTime_(),
	  poller_(Poller::newDefaultPoller(this)),
	  wakeupFd_(createNonblockingEventfdOrDie()),
	  wakeupChannel_(new Channel(this, wakeupFd_)),
	  currentActiveChannel_(nullptr) {
  LOG_DEBUG << "created " << this << " in thread " << threadId_;
  if (loopInThisThread_ts) {
	LOG_FATAL << "Another EventLoop " << loopInThisThread_ts
			  << " exits in this thread " << threadId_;
  } else {
	loopInThisThread_ts = this;
  }
  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
  LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
			<< " destructs in thread " << gettid();
  // 那么其他的通道是怎么处理的？
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  loopInThisThread_ts = nullptr;
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;
  LOG_TRACE << "EventLoop " << this << " start looping";

  while (!quit_) {
	activeChannels_.clear();
	pollReturnTime_ = poller_->poll(&activeChannels_, kPollTimeMs);
	++iteration_;
	if (Logger::logLevel() <= Logger::TRACE)
	  printActiveChannels();

	eventHandling_ = true;
	for (auto &channel:activeChannels_) {
	  currentActiveChannel_ = channel;
	  currentActiveChannel_->handleEvent(pollReturnTime_);
	}
	currentActiveChannel_ = nullptr;
	eventHandling_ = false;
	doPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  LOG_TRACE<<"before quit = true";
  quit_ = true;
  LOG_TRACE<<"after quit = true";
  if (!isInLoopThread())
	wakeup();
}

void EventLoop::runInLoop(Functor cb) {
  LOG_TRACE<<"gettid() = "<<gettid()<<", threadId_: "<<threadId_;
  if (isInLoopThread()) {
    LOG_TRACE<<"cb()";
	cb();
  }
  else
	queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor cb) {
  {
	std::lock_guard<std::mutex> lock(mutex_);
	pendingFunctors_.push_back(std::move(cb));
  }
  if (!isInLoopThread() || callingPendingFunctors_)
	wakeup();
}

void EventLoop::updateChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (eventHandling_) {
	// for what?
	assert(currentActiveChannel_ == channel ||
		std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
  }
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  return poller_->hasChannel(channel);
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one))
	LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
}

size_t EventLoop::queueSize() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return pendingFunctors_.size();
}

void EventLoop::assertInLoopThread() {
  if (!isInLoopThread())
	abortNotInLoopThread();
}

bool EventLoop::isInLoopThread() const {
  return gettid() == threadId_;
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
  return loopInThisThread_ts;
}

void EventLoop::abortNotInLoopThread() {
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop" << this
			<< " was create in threadId_ = " << threadId_
			<< ", current thread id = " << ::gettid();
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one))
	LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
	std::lock_guard<std::mutex> lock(mutex_);
	functors.swap(pendingFunctors_);
  }

  for (const auto &functor:functors)
	functor();
  callingPendingFunctors_ = false;
  LOG_TRACE<<"PendingFunctors done";
}

void EventLoop::printActiveChannels() const {
  for (const auto &channel:activeChannels_)
	LOG_TRACE << "{" << channel->reventsToString() << "}";
}