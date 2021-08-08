//
// Created by Ye-zixiao on 2021/4/7.
//

#include "libfm/fmnet/EventLoop.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <algorithm>
#include <csignal>
#include <cassert>
#include "libfm/fmutil/Log.h"
#include "libfm/fmnet/Channel.h"
#include "net/SocketsOps.h"
#include "net/Epoller.h"
#include "net/TimerQueue.h"
#include "net/Timer.h"

using namespace fm;
using namespace fm::net;

namespace {

thread_local EventLoop *loop_in_this_thread_ts = nullptr;
constexpr int kPollTimeMs = 10000; // 10s

int createNonblockingEventfdOrDie() {
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) LOG_FATAL << "Failed in eventfd";
  return evtfd;
}

struct IgnoreSigPipe {
  IgnoreSigPipe() {
    ::signal(SIGPIPE, SIG_IGN);
  }
};

// 程序创建之初就忽略SIGPIPE信号
IgnoreSigPipe g_init_to_ignore_sigpipe;

} // unnamed namespace

EventLoop::EventLoop()
    : looping_(false),
      quit_(false),
      event_handling_(false),
      calling_pending_functors_(false),
      iteration_(0),
      thread_id_(gettid()),
      poll_return_time_(),
      poller_(std::make_unique<Epoller>(this)),
      timer_queue_(std::make_unique<TimerQueue>(this)),
      wakeup_fd_(createNonblockingEventfdOrDie()),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      current_active_channel_(nullptr) {
  LOG_DEBUG << "created " << this << " in thread " << thread_id_;

  // 检查当前线程是否已经运行了事件循环，若是直接终止进程
  if (loop_in_this_thread_ts) {
    LOG_FATAL << "Another EventLoop " << loop_in_this_thread_ts
              << " exits in this thread " << thread_id_;
  } else {
    loop_in_this_thread_ts = this;
  }
  // 将唤醒频道注册到事件循环中
  wakeup_channel_->setReadCallback([this](time::TimeStamp t) { handleRead(t); });
  wakeup_channel_->enableReading();
}

EventLoop::~EventLoop() {
  wakeup_channel_->disableAll();
  wakeup_channel_->remove();
  ::close(wakeup_fd_);
  loop_in_this_thread_ts = nullptr;
}

void EventLoop::loop() {
  assert(!looping_);
  assertInLoopThread();
  looping_ = true;
  quit_ = false;
  LOG_DEBUG << "EventLoop " << this << " start looping";

  while (!quit_) {
    active_channels_.clear();
    // 通过Poller中的epoll_wait()或poll()等待激活事件的通知
    poll_return_time_ = poller_->poll(&active_channels_, kPollTimeMs);
    ++iteration_;
    if (log::currentLogLevel() >= log::LogLevel::kINFO)
      printActiveChannels();

    // 开始处理激活频道上的回调函数
    event_handling_ = true;
    for (auto &channel:active_channels_) {
      current_active_channel_ = channel;
      current_active_channel_->handleEvent(poll_return_time_);
    }
    current_active_channel_ = nullptr;
    event_handling_ = false;

    // 特别处理事件循环自己的唤醒频道上的可读事件
    doPendingFunctors();
  }

  LOG_DEBUG << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread())
    wakeup();
}

void EventLoop::runInLoop(Functor cb) {
  if (isInLoopThread()) cb();
  else queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor cb) {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pending_functors_.push_back(std::move(cb));
  }
  if (!isInLoopThread() || calling_pending_functors_)
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
  if (event_handling_) {
    // 当前事件处理器的频道想移除自己或者触发可用频道没有自己，那么直接终止进程
    assert(current_active_channel_ == channel ||
        std::find(active_channels_.begin(), active_channels_.end(), channel)
            == active_channels_.end());
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
  ssize_t n = sockets::write(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof(one))
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
}

size_t EventLoop::queueSize() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return pending_functors_.size();
}

void EventLoop::assertInLoopThread() {
  if (!isInLoopThread())
    abortNotInLoopThread();
}

bool EventLoop::isInLoopThread() const {
  return gettid() == thread_id_;
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
  return loop_in_this_thread_ts;
}

void EventLoop::abortNotInLoopThread() {
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop" << this
            << " was create in thread_id_ = " << thread_id_
            << ", current thread id = " << ::gettid();
}

void EventLoop::handleRead(time::TimeStamp) {
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeup_fd_, &one, sizeof(one));
  if (n != sizeof(one))
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  calling_pending_functors_ = true;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    functors.swap(pending_functors_);
  }

  for (const auto &functor:functors)
    functor();
  calling_pending_functors_ = false;
  LOG_DEBUG << "PendingFunctors done";
}

void EventLoop::printActiveChannels() const {
  for (const auto &channel:active_channels_)
    LOG_DEBUG << "{" << channel->reventsToString() << "}";
}

TimerId EventLoop::runAt(time::TimeStamp time, TimerCallback cb) {
  using duration = time::TimeStamp::DefaultDuration;
  return timer_queue_->addTimer(std::move(cb),
                                time,
                                duration::zero());
}

TimerId EventLoop::runAfter(const time::TimeStamp::DefaultDuration &interval,
                            TimerCallback cb) {
  return timer_queue_->addTimer(std::move(cb),
                                time::SystemClock::now() + interval,
                                time::TimeStamp::DefaultDuration::zero());
}

TimerId EventLoop::runEvery(const time::TimeStamp::DefaultDuration &interval,
                            TimerCallback cb) {
  return timer_queue_->addTimer(std::move(cb),
                                time::SystemClock::now() + interval,
                                interval);
}

void EventLoop::cancel(TimerId timerId) {
  timer_queue_->delTimer(timerId);
}