//
// Created by Ye-zixiao on 2021/4/9.
//

#ifndef LIBFM_NET_EVENTLOOPTHREADPOOL_H_
#define LIBFM_NET_EVENTLOOPTHREADPOOL_H_

#include <string>
#include <vector>
#include <memory>

#include "libfm/base/noncoapyable.h"

namespace fm::net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : private noncopyable {
 public:
  EventLoopThreadPool(EventLoop *baseLoop, const std::string &name);
  ~EventLoopThreadPool() = default;

  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start();

  EventLoop *getNextLoop();
  EventLoop *getLoopForHash(size_t hashCode);
  std::vector<EventLoop *> getAllLoops();

  bool isStarted() const { return start_; }
  const std::string& name() const { return name_; }

 private:
  EventLoop *baseLoop_;
  std::string name_;
  bool start_;
  int numThreads_;
  int next_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop *> loops_;
};

} // namespace fm::net

#endif //LIBFM_NET_EVENTLOOPTHREADPOOL_H_
