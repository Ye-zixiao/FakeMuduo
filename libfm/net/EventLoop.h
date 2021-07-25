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
#include "libfm/base/NonCopyable.h"
#include "libfm/base/Timestamp.h"
#include "libfm/net/Callback.h"
#include "libfm/net/TimerQueue.h"

namespace fm::net {

class Channel;
class Epoller;
class TimerId;
class TimerQueue;

class EventLoop : private NonCopyable {
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

  // 本来是想让runAfter和runEvery这两个接口通过模板的方式进行实现，但后来想
  // 了想还是算了，这并不是因为性能的问题，主要是考虑代码的美观性而且还要放入头
  // 文件。因此我们觉得仅仅使用单一的接口通过duration自身的类类型转换是足够的
//  template<typename Rep, typename Period>
//  TimerId runAfter(const std::chrono::duration<Rep, Period> &interval,
//                   TimerCallback cb);
//  template<typename Rep, typename Period>
//  TimerId runEvery(const std::chrono::duration<Rep, Period> &interval,
//                   TimerCallback cb);
  TimerId runAt(time::Timestamp time, TimerCallback cb);
  TimerId runAfter(const time::Timestamp::duration &interval,
                   TimerCallback cb);
  TimerId runEvery(const time::Timestamp::duration &interval,
                   TimerCallback cb);
  void cancel(TimerId timerId);

  size_t queueSize() const;
  time::Timestamp pollReturnTime() { return poll_return_time_; }
  int64_t iteration() const { return iteration_; }
  pid_t threadId() const { return thread_id_; }

  void assertInLoopThread();
  bool isInLoopThread() const;

  static EventLoop *getEventLoopOfCurrentThread();

 private:
  void abortNotInLoopThread();
  void handleRead(time::Timestamp);
  void doPendingFunctors();

  void printActiveChannels() const;

 private:
  using ChannelList = std::vector<Channel *>;

  std::atomic_bool looping_;
  std::atomic_bool quit_;
  std::atomic_bool event_handling_;
  std::atomic_bool calling_pending_functors_;

  int64_t iteration_;
  const pid_t thread_id_;
  time::Timestamp poll_return_time_;

  std::unique_ptr<Epoller> poller_;          // 轮询器
  std::unique_ptr<TimerQueue> timer_queue_;  // 定时器队列

  int wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;  // 其他线程唤醒I/O线程的通道

  ChannelList active_channels_;
  Channel *current_active_channel_;

  mutable std::mutex mutex_;
  std::vector<Functor> pending_functors_;
};

} // namespace fm::net

#endif //LIBFM_NET_EVENTLOOP_H_
