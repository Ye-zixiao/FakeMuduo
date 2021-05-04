//
// Created by Ye-zixiao on 2021/4/8.
//

#include "../Poller.h"

#include "EpollPoller.h"

using namespace fm::net;

Poller* Poller::newDefaultPoller(EventLoop *loop) {
  return new EpollPoller(loop);
}