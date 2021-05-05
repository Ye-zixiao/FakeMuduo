//
// Created by Ye-zixiao on 2021/5/5.
//

#ifndef FAKEMUDUO_SRC_NET_TIMERQUEUE_H_
#define FAKEMUDUO_SRC_NET_TIMERQUEUE_H_

#include <set>

#include "../base/noncoapyable.h"
#include "../base/TimeStamp.h"
#include "Callback.h"
#include "Channel.h"

namespace fm {
namespace net {

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : private noncopyable {
 public:
  explicit TimerQueue(EventLoop *loop);
  ~TimerQueue();

  TimerId addTimer(TimerCallback cb, TimeStamp time, double interval);
  void delTimer(TimerId timerId);

 private:
  /**
   * 原作者说对于Timer定时器使用原始指针不太好，应该试图使用unique_ptr。
   * 但实际上unique_ptr在这里使用也不是很方便，shared_ptr+weak_ptr
   * 可能会好一点，但是性能上又有点得不偿失。。。
   */
  using TimerEntry = std::pair<TimeStamp, Timer *>;
  using TimerTree = std::set<TimerEntry>;
  using TimerIdSet = std::set<TimerId>; // 怎么设置成unordered_set？

  void handleRead();

  void addTimerInLoop(Timer *timer);
  void delTimerInLoop(TimerId timerId);

  void resetTimer(const std::vector<TimerEntry> &expired, TimeStamp now);
  bool insertTimer(Timer *timer);

  std::vector<TimerEntry> getExpired(TimeStamp now);

 private:
  EventLoop *loop_;
  const int timerFd_;         // 注意timerFd在系统内只能为用户维护一个定时器
  Channel timerFdChannel_;

  TimerTree timerTree_;       // 定时器红黑树，按照过期时间进行排序
  TimerIdSet activeTimers_;   // 定时器标识集合，记录哪些定时器仍然有效/活跃
  TimerIdSet deletedTimers_;  // 定时器标识集合，记录哪些定时器即将被删除
  bool callingExpiredTimers_; // 当前是否在调用用户的定时器回调函数
};

} // namespace net
} // namespace fm

#endif //FAKEMUDUO_SRC_NET_TIMERQUEUE_H_
