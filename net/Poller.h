//
// Created by Ye-zixiao on 2021/4/8.
//

#ifndef FAKEMUDUO_NET_POLLER_H_
#define FAKEMUDUO_NET_POLLER_H_

#include <unordered_map>
#include <vector>

#include "../base/noncoapyable.h"
#include "../base/TimeStamp.h"
#include "EventLoop.h"

namespace fm {

namespace net {

class Channel;
class EventLoop;

class Poller : private noncopyable {
 public:
  using ChannelList = std::vector<Channel *>;

  explicit Poller(EventLoop *loop)
	  : ownerLoop_(loop) {}

  virtual ~Poller() = default;

  virtual TimeStamp poll(ChannelList *activateChannels, int timeoutMs) = 0;

  virtual void updateChannel(Channel *channel) = 0;
  virtual void removeChannel(Channel *channel) = 0;
  virtual bool hasChannel(Channel *channel) const;

  static Poller *newDefaultPoller(EventLoop *loop);

  void assertInLoopThread() const { ownerLoop_->assertInLoopThread(); }

 protected:
  using ChannelMap = std::unordered_map<int, Channel *>;
  ChannelMap channels_;

 private:
  EventLoop *ownerLoop_;

};

} // namespace net

} // namespace fm

#endif //FAKEMUDUO_NET_POLLER_H_
