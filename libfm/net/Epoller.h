//
// Created by Ye-zixiao on 2021/6/6.
//

#ifndef LIBFM_NET_EPOLLER_H_
#define LIBFM_NET_EPOLLER_H_

#include <sys/epoll.h>
#include <unordered_map>
#include <vector>
#include "libfm/base/NonCopyable.h"
#include "libfm/base/Timestamp.h"

namespace fm::net {

class Channel;
class EventLoop;

class Epoller : private NonCopyable {
 public:
  using ChannelList = std::vector<Channel *>;

  explicit Epoller(EventLoop *loop);
  ~Epoller();

  time::Timestamp poll(ChannelList *activateChannels, int timeoutMs);

  void updateChannel(Channel *channel);
  void removeChannel(Channel *channel);
  bool hasChannel(Channel *channel) const;

  void assertInLoopThread() const;

 private:
  static constexpr int kInitEventListSize = 16;
  static const char *operationToString(int op);

  void fillActiveChannels(ChannelList *activeChannels, int numEvents) const;

  void update(Channel *channel, int operation);

 private:
  using ChannelMap = std::unordered_map<int, Channel *>;
  using EventList = std::vector<struct epoll_event>;

  EventLoop *owner_loop_;
  ChannelMap channels_;  // 保存文件描述符fd到频道Channel指针的映射
  int epoll_fd_;
  EventList events_;
};

} // namespace fm::net;

#endif //LIBFM_NET_EPOLLER_H_
