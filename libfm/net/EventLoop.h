//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef LIBFM_NET_EVENTLOOP_H_
#define LIBFM_NET_EVENTLOOP_H_

#include <atomic>
#include <functional>
#include <vector>
#include <memory>
#include <mutex>

#include "libfm/base/noncoapyable.h"
#include "libfm/base/Timestamp1.h"
#include "libfm/net/Callback.h"

namespace fm::net {

class Channel;
class Epoller;
class TimerId;
class TimerQueue;

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

  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel);

  // 定时器部分先不实现
  TimerId runAt(time::Timestamp time, TimerCallback cb);
//  template<typename Rep, typename Period>
//  TimerId runAfter(const std::chrono::duration<Rep, Period> &interval,
//                   TimerCallback cb);
//  template<typename Rep, typename Period>
//  TimerId runEvery(const std::chrono::duration<Rep, Period> &interval,
//                   TimerCallback cb);
  TimerId runAfter(const time::Timestamp::duration &interval,
                   TimerCallback cb);
  TimerId runEvery(const time::Timestamp::duration &interval,
                   TimerCallback cb);
  void cancel(TimerId timerId);

  size_t queueSize() const;
  time::Timestamp pollReturnTime() { return pollReturnTime_; }
  int64_t iteration() const { return iteration_; }
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
  time::Timestamp pollReturnTime_;

  std::unique_ptr<Epoller> poller_;         // 轮询器
  std::unique_ptr<TimerQueue> timerQueue_; // 定时器队列

  int wakeupFd_;
  std::unique_ptr<Channel> wakeupChannel_; // 其他线程唤醒I/O线程的通道

  ChannelList activeChannels_;
  Channel *currentActiveChannel_;

  mutable std::mutex mutex_;
  std::vector<Functor> pendingFunctors_;
};

} // namespace fm::net

#endif //LIBFM_NET_EVENTLOOP_H_
