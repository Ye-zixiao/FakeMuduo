//
// Created by Ye-zixiao on 2021/4/8.
//

#ifndef FAKEMUDUO_NET_POLLER_EPOLLPOLLER_H_
#define FAKEMUDUO_NET_POLLER_EPOLLPOLLER_H_

#include "../Poller.h"

#include <vector>

struct epoll_event;

namespace fm {

namespace net {

class EpollPoller : public Poller {
 public:
  explicit EpollPoller(EventLoop* loop);
  ~EpollPoller() override;

  TimeStamp poll(ChannelList* activeChannels,int timeoutMs) override;

  void updateChannel(Channel* channel) override;
  void removeChannel(Channel* channel) override;

 private:
  static const int kInitEventListSize = 16;
  static const char *operationToString(int op);

  void fillActiveChannels(ChannelList* activeChannels,int numEvents) const;

  void update(Channel *channel,int operation);

 private:
  using EventList = std::vector<struct epoll_event>;

  int epollfd_;
  EventList events_; // 暂时存储epoll_wait的返回事件集
};

} // namespace net

} // namespace fm

#endif //FAKEMUDUO_NET_POLLER_EPOLLPOLLER_H_
