//
// Created by Ye-zixiao on 2021/5/5.
//

#include "libfm/net/TimerQueue.h"

#include <sys/timerfd.h>
#include <unistd.h>
#include <cassert>
#include <algorithm>
#include <vector>

#include "libfm/base/Logging.h"
#include "libfm/net/EventLoop.h"

using namespace fm;
using namespace fm::net;

namespace {

/** ====================================
 * 定义一些关于timerfd（定时器描述符）的辅助函数
 *  ====================================
 */

int createTimerfd() {
  int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
                                 TFD_NONBLOCK | TFD_CLOEXEC);
  LOG_TRACE << "create timerfd " << timerfd;
  if (timerfd < 0)
    LOG_SYSFATAL << "Failed in timerfd_create()";
  return timerfd;
}

struct timespec timespecFromNow(time::Timestamp when) {
  timespec interval = time::SystemClock::to_timespec(when - time::SystemClock::now());
  if (interval.tv_sec < 0)
    interval.tv_sec = 0;
  if (interval.tv_sec == 0 && interval.tv_nsec < 100)
    interval.tv_nsec = 100;
  return interval;
}

void readTimerfd(int timerfd, time::Timestamp now) {
  uint64_t num;
  ssize_t nRead = ::read(timerfd, &num, sizeof(num));
  LOG_TRACE << "TimerQueue::handleRead() " << num <<
            " at " << now.toString(true);
  if (nRead != sizeof(num))
    LOG_ERROR << "TimerQueue::handleRead() reads "
              << nRead << " bytes instead of 8";
}

void updateTimerfd(int timerfd, time::Timestamp expiration) {
  struct itimerspec newValue{};
  // itimerspec结构体中的重复时间间隔字段不设置，重复定时由TimerQueue自己实现
  newValue.it_value = timespecFromNow(expiration);
  int ret = ::timerfd_settime(timerfd, 0, &newValue, nullptr);
  if (ret < 0)
    LOG_ERROR << "timerfd_settime()";
  LOG_TRACE << "timerfd_settime success";
}

} // unnamed namespace

TimerQueue::TimerQueue(EventLoop *loop)
    : loop_(loop),
      timerFd_(createTimerfd()),
      timerFdChannel_(loop, timerFd_),
      callingExpiredTimers_(false) {
  timerFdChannel_.setReadCallback(
      std::bind(&TimerQueue::handleRead, this));
  timerFdChannel_.enableReading();
}

TimerQueue::~TimerQueue() {
  timerFdChannel_.disableAll();
  timerFdChannel_.remove();
  ::close(timerFd_);
  for (auto &entry:timerTree_)
    delete entry.second;
}

TimerId TimerQueue::addTimer(TimerCallback cb, time::Timestamp time, duration interval) {
  auto *timer = new Timer(std::move(cb), time, interval);
  loop_->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
  return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer *timer) {
  loop_->assertInLoopThread();
  bool isEarliestTimer = insertTimer(timer);
  // 因为timerFd只能设置一个未来的定时时间，不能设置多于一个的，
  // 所以我们只能为定时器队列设置最近的定时时间，只有等到定时时间
  // 过期后通过重置的方式来实现多个定时器的设置
  if (isEarliestTimer)
    updateTimerfd(timerFd_, timer->expiration());
}

void TimerQueue::delTimer(TimerId timerId) {
  loop_->runInLoop(std::bind(&TimerQueue::delTimerInLoop, this, timerId));
}

void TimerQueue::delTimerInLoop(TimerId timerId) {
  loop_->assertInLoopThread();
  assert(timerTree_.size() == activeTimers_.size());

  auto iter = activeTimers_.find(timerId);
  if (iter != activeTimers_.end()) {
    Timer *timer = timerId.timer();
    // 先从定时器红黑树中删除关于这个定时器的表项记录
    size_t n = timerTree_.erase(TimerEntry(timer->expiration(), timer));
    assert(n == 1);
    (void)n;
    // 再从活跃定时器集合中删除这个定时器
    activeTimers_.erase(iter);
    delete timer;
  } else if (callingExpiredTimers_) {
    // 如果没在活跃定时器集合中找到，可能说明这个定时器正好过期正在执行回调函数而放
    // 在expired过期定时器表项vector中，那么我们此时需要将其放入到deletedTimer
    // 失效定时器标识集合中，等待resetTimer()执行时进行销毁
    deletedTimers_.insert(timerId);
  }
  assert(timerTree_.size() == activeTimers_.size());
}

void TimerQueue::handleRead() {
  loop_->assertInLoopThread();
  time::Timestamp now(time::SystemClock::now());
  readTimerfd(timerFd_, now);

  // 获取过期定时器集合，并回调它们中的用户函数
  // 应该不存在关于callingExpiredTimers_的线程安全问题
  std::vector<TimerEntry> expiredTimers = getExpired(now);
  callingExpiredTimers_ = true;
  deletedTimers_.clear();
  for (const auto &entry:expiredTimers)
    entry.second->run();
  callingExpiredTimers_ = false;

  // 重置timerFd中的定时时间
  resetTimer(expiredTimers, now);
}

void TimerQueue::resetTimer(const std::vector<TimerEntry> &expired, time::Timestamp now) {
  for (const TimerEntry &entry:expired) {
    TimerId timerId(entry.second, entry.second->sequence());
    // 将那些需要周期定时的定时器重设，而那些以及过期且只设置一次的定时器进行销毁
    if (entry.second->isRepeated() &&
        deletedTimers_.find(timerId) == deletedTimers_.end()) {
      entry.second->restart(now);
      insertTimer(entry.second);
    } else {
      delete entry.second;
    }
  }

  // 如果定时器树上还有定时器，说明还需要让timerfd设置新的定时
  if (!timerTree_.empty()) {
    auto nextExpire = timerTree_.begin()->second->expiration();
    updateTimerfd(timerFd_, nextExpire);
  }
}

bool TimerQueue::insertTimer(Timer *timer) {
  loop_->assertInLoopThread();
  assert(timerTree_.size() == activeTimers_.size());

  // 检查当前插入的定时器的到期时间是否早于定时器树
  // （有序）中的所有定时器的到期时间
  bool isEarliestTimer = false;
  time::Timestamp when = timer->expiration();
  if (timerTree_.empty() || when < timerTree_.begin()->first)
    isEarliestTimer = true;

  // 分别插入到定时器列表和活跃定时器列表中
  auto res1 = timerTree_.insert(TimerEntry(timer->expiration(), timer));
  assert(res1.second);
  (void)res1;
  auto res2 = activeTimers_.emplace(timer, timer->sequence());
  assert(res2.second);
  (void)res2;

  return isEarliestTimer;
}

std::vector<TimerQueue::TimerEntry> TimerQueue::getExpired(time::Timestamp now) {
  assert(timerTree_.size() == activeTimers_.size());

  // 找出定时器列表（实际上是一颗红黑树）中已经失效的定时器，将它们加入到
  // 失效定时器集合中，并从TimerQueue中的定时器列表中删除它们
  std::vector<TimerEntry> expired;
  TimerEntry entryGuard(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
  auto iter = timerTree_.lower_bound(entryGuard);
  assert(iter == timerTree_.end() || now < iter->first);
  std::copy(timerTree_.begin(), iter, std::back_inserter(expired));
  timerTree_.erase(timerTree_.begin(), iter);

  // 从活跃定时器标识集合中删除过期的定时器标识
  for (const TimerEntry &entry:expired) {
    TimerId timerId(entry.second, entry.second->sequence());
    size_t n = activeTimers_.erase(timerId);
    assert(n == 1);
    (void)n;
  }
  assert(timerTree_.size() == activeTimers_.size());
  return expired;
}