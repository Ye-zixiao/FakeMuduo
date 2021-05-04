//
// Created by Ye-zixiao on 2021/4/9.
//

#ifndef FAKEMUDUO_NET_EVENTLOOPTHREAD_H_
#define FAKEMUDUO_NET_EVENTLOOPTHREAD_H_

#include <condition_variable>
#include <memory>
#include <thread>
#include <mutex>

#include "../base/noncoapyable.h"

namespace fm {
namespace net {

class EventLoop;

class EventLoopThread : private noncopyable {
 public:
  EventLoopThread(const std::string &name = std::string());
  ~EventLoopThread();

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

} // namespace net
} // namespace fm

#endif //FAKEMUDUO_NET_EVENTLOOPTHREAD_H_
