//
// Created by Ye-zixiao on 2021/4/9.
//

#ifndef LIBFM_NET_EVENTLOOPTHREADPOOL_H_
#define LIBFM_NET_EVENTLOOPTHREADPOOL_H_

#include <string>
#include <vector>
#include <memory>

namespace fm::net {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
 public:
  EventLoopThreadPool(EventLoop *baseLoop, std::string name);
  ~EventLoopThreadPool() = default;

  EventLoopThreadPool(const EventLoopThreadPool &) = delete;
  EventLoopThreadPool &operator=(const EventLoopThreadPool &) = delete;

  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start();

  EventLoop *getNextLoop();
  EventLoop *getLoopForHash(size_t hashCode);
  std::vector<EventLoop *> getAllLoops();

  bool isStarted() const { return start_; }
  std::string_view name() const { return name_; }

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
