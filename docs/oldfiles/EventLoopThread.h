//
// Created by Ye-zixiao on 2021/4/9.
//

#ifndef LIBFM_NET_EVENTLOOPTHREAD_H_
#define LIBFM_NET_EVENTLOOPTHREAD_H_

#include <condition_variable>
#include <memory>
#include <thread>
#include <mutex>

namespace fm::net {

class EventLoop;

class EventLoopThread {
 public:
  EventLoopThread();
  ~EventLoopThread();

  EventLoopThread(const EventLoopThread &) = delete;
  EventLoopThread &operator=(const EventLoopThread &) = delete;

  EventLoop *startLoop();

 private:
  void threadFunc();

 private:
  EventLoop *loop_;
  bool exiting_;
  std::unique_ptr<std::thread> thread_;
  std::mutex mutex_;
  std::condition_variable cond_;
};

} // namespace fm::net

#endif //LIBFM_NET_EVENTLOOPTHREAD_H_
