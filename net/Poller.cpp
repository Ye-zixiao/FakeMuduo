//
// Created by Ye-zixiao on 2021/4/8.
//

#include "Poller.h"

#include "Channel.h"

using namespace fm::net;

bool Poller::hasChannel(Channel *channel) const {
  assertInLoopThread();
  auto iter = channels_.find(channel->fd());
  return iter != channels_.end() && iter->second == channel;
}