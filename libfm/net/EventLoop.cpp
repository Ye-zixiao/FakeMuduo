//
// Created by Ye-zixiao on 2021/4/7.
//

#include "libfm/net/EventLoop.h"

#include <sys/eventfd.h>
#include <unistd.h>
#include <algorithm>
#include <csignal>
#include <cassert>

#include "libfm/base/Logging.h"
#include "libfm/net/SocketsOps.h"
#include "libfm/net/TimerQueue.h"
#include "libfm/net/Epoller.h"

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
      poller_(std::make_unique<Epoller>(this)),
      timerQueue_(std::make_unique<TimerQueue>(this)),
      wakeupFd_(createNonblockingEventfdOrDie()),
      wakeupChannel_(new Channel(this, wakeupFd_)),
      currentActiveChannel_(nullptr) {
  LOG_DEBUG << "created " << this << " in thread " << threadId_;

  // 检查当前线程是否已经运行了事件循环，若是直接终止进程
  if (loopInThisThread_ts) {
    LOG_FATAL << "Another EventLoop " << loopInThisThread_ts
              << " exits in this thread " << threadId_;
  } else {
    loopInThisThread_ts = this;
  }
  // 将唤醒频道注册到事件循环中
  wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead, this));
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
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
    // 通过Poller中的epoll_wait()或poll()等待激活事件的通知
    pollReturnTime_ = poller_->poll(&activeChannels_, kPollTimeMs);
    ++iteration_;
    if (Logger::logLevel() <= Logger::TRACE)
      printActiveChannels();

    // 开始处理激活频道上的回调函数
    eventHandling_ = true;
    for (auto &channel:activeChannels_) {
      currentActiveChannel_ = channel;
      currentActiveChannel_->handleEvent(pollReturnTime_);
    }
    currentActiveChannel_ = nullptr;
    eventHandling_ = false;

    // 特别处理事件循环自己的唤醒频道上的可读事件
    doPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread())
    wakeup();
}

void EventLoop::runInLoop(Functor cb) {
  if (isInLoopThread()) {
    cb();
  } else
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
    // 当前事件处理器的频道想移除自己或者触发可用频道没有自己，那么直接终止进程
    assert(currentActiveChannel_ == channel ||
        std::find(activeChannels_.begin(), activeChannels_.end(), channel)
            == activeChannels_.end());
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
  LOG_TRACE << "PendingFunctors done";
}

TimerId EventLoop::runAt(time::Timestamp time, TimerCallback cb) {
  using duration = time::Timestamp::duration;
  return timerQueue_->addTimer(std::move(cb),
                               time,
                               duration::zero());
}

//template<typename Rep, typename Period>
//TimerId EventLoop::runAfter(const std::chrono::duration<Rep, Period> &interval,
//                            TimerCallback cb) {
//  using duration = time::Timestamp::duration;
//  return timerQueue_->addTimer(std::move(cb),
//                               time::SystemClock::now() + interval,
//                               duration::zero());
//}
//
//template<typename Rep, typename Period>
//TimerId EventLoop::runEvery(const std::chrono::duration<Rep, Period> &interval,
//                            TimerCallback cb) {
//  return timerQueue_->addTimer(std::move(cb),
//                               time::SystemClock::now() + interval,
//                               interval);
//}

TimerId EventLoop::runAfter(const time::Timestamp::duration &interval,
                            TimerCallback cb) {
  return timerQueue_->addTimer(std::move(cb),
                               time::SystemClock::now() + interval,
                               time::Timestamp::duration::zero());
}

TimerId EventLoop::runEvery(const time::Timestamp::duration &interval,
                            TimerCallback cb) {
  return timerQueue_->addTimer(std::move(cb),
                               time::SystemClock::now() + interval,
                               interval);
}

void EventLoop::cancel(TimerId timerId) {
  timerQueue_->delTimer(timerId);
}

//TimerId EventLoop::runAt(TimeStamp time, TimerCallback cb) {
//  return timerQueue_->addTimer(std::move(cb), time, 0);
//}
//
//TimerId EventLoop::runAfter(double delay, TimerCallback cb) {
//  TimeStamp time(timeAdd(TimeStamp::now(), delay));
//  return timerQueue_->addTimer(std::move(cb), time, 0);
//}
//
//TimerId EventLoop::runEvery(double interval, TimerCallback cb) {
//  TimeStamp time(timeAdd(TimeStamp::now(), interval));
//  return timerQueue_->addTimer(std::move(cb), time, interval);
//}
//
//void EventLoop::cancel(TimerId timerId) {
//  timerQueue_->delTimer(timerId);
//}

void EventLoop::printActiveChannels() const {
  for (const auto &channel:activeChannels_)
    LOG_TRACE << "{" << channel->reventsToString() << "}";
}