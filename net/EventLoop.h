//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef FAKEMUDUO_NET_EVENTLOOP_H_
#define FAKEMUDUO_NET_EVENTLOOP_H_

#include <atomic>
#include <functional>
#include <vector>
#include <memory>
#include <mutex>

#include "../base/noncoapyable.h"
#include "../base/TimeStamp.h"
#include "Callback.h"

namespace fm {

namespace net {

class Channel;
class Poller;
//class TimerQueue;
//class TimerId;

class EventLoop : private noncopyable {
 public:
  using Functor = std::function<void()>;

  EventLoop();
  ~EventLoop();

  void loop();
  void quit();
  void wakeup();

  void runInLoop(Functor cb);
  void queueInLoop(Functor cb);

  // 定时器部分先不实现
//  TimerId runAt(TimeStamp time, TimerCallback cb);
//  TimerId runAfter(double delay, TimerCallback cb);
//  TimerId runEvery(double interval, TimerCallback cb);
//  void cancel(TimerId timerId);

  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel);

  size_t queueSize() const;
  TimeStamp pollReturnTime() { return pollReturnTime_; }
  int64_t iteratoion() const { return iteration_; }
  pid_t threadId() const { return threadId_; }

  void assertInLoopThread();
  bool isInLoopThread() const;

  static EventLoop *getEventLoopOfCurrentThread();

 private:
  void abortNotInLoopThread();
  void handleRead();
  void doPendingFunctors();

  void printActiveChannels() const;

 private:
  using ChannelList = std::vector<Channel *>;

  std::atomic_bool looping_;
  std::atomic_bool quit_;
  std::atomic_bool eventHandling_;
  std::atomic_bool callingPendingFunctors_;

  int64_t iteration_;
  const pid_t threadId_;
  TimeStamp pollReturnTime_;

  std::unique_ptr<Poller> poller_;         // 轮询器
//  std::unique_ptr<TimerQueue> timerQueue_; // 定时器队列

  int wakeupFd_;
  std::unique_ptr<Channel> wakeupChannel_; // 其他线程唤醒I/O线程的通道

  ChannelList activeChannels_;
  Channel *currentActiveChannel_;

  mutable std::mutex mutex_;
  std::vector<Functor> pendingFunctors_;
};

} // namespace net

} // namespace fm

#endif //FAKEMUDUO_NET_EVENTLOOP_H_
