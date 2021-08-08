//
// Created by Ye-zixiao on 2021/6/6.
//

#include "net/Epoller.h"
#include <cassert>
#include <cerrno>
#include <unistd.h>
#include "libfm/fmutil/Log.h"
#include "libfm/fmnet/EventLoop.h"
#include "libfm/fmnet/Channel.h"

using namespace std;
using namespace fm;
using namespace fm::net;

namespace {
constexpr int kNew = -1;
constexpr int kAdded = 1;
constexpr int kDeleted = 2;
} // unnamed namespace

Epoller::Epoller(EventLoop *loop)
    : owner_loop_(loop),
      epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
  if (epoll_fd_ < 0)
    LOG_FATAL << "Epoller::Epoller";
}

Epoller::~Epoller() {
  ::close(epoll_fd_);
}

time::TimeStamp Epoller::poll(ChannelList *activateChannels, int timeoutMs) {
  LOG_DEBUG << "fd total count: " << channels_.size();
  int numEvents = ::epoll_wait(epoll_fd_,
                               events_.data(),
                               static_cast<int>(events_.size()),
                               timeoutMs);
  int savedErrno = errno;
  time::TimeStamp now(time::SystemClock::now());

  if (numEvents > 0) {
    LOG_DEBUG << numEvents << " events happened";
    fillActiveChannels(activateChannels, numEvents);
    if (numEvents == static_cast<int>(events_.size()))
      events_.resize(events_.size() * 2);
  } else if (numEvents == 0) {
    LOG_DEBUG << "nothing happend";
  } else if (savedErrno != EINTR) {
    errno = savedErrno;
    LOG_FATAL << "Epoller::poll";
  }
  return now;
}

void Epoller::updateChannel(Channel *channel) {
  assertInLoopThread();

  const int status = channel->index();
  const int fd = channel->fd();
  LOG_DEBUG << "fd = " << fd << " events = {"
            << channel->eventsToString() << "} status " << status;

  if (status == kNew || status == kDeleted) {
    if (status == kNew) {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    } else {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    channel->set_index(kAdded);
    update(channel, EPOLL_CTL_ADD);
  } else {
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    if (channel->isNoneEvent()) {
      update(channel, EPOLL_CTL_DEL);
      channel->set_index(kDeleted);
    } else {
      update(channel, EPOLL_CTL_MOD);
    }
  }
}

void Epoller::removeChannel(Channel *channel) {
  assertInLoopThread();

  const int fd = channel->fd();
  const int status = channel->index();
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  assert(status == kAdded || status == kDeleted);
  size_t n = channels_.erase(fd);
  assert(n == 1);
  (void)n;

  if (status == kAdded)
    update(channel, EPOLL_CTL_DEL);
  channel->set_index(kNew);
}

bool Epoller::hasChannel(Channel *channel) const {
  assertInLoopThread();
  auto iter = channels_.find(channel->fd());
  return iter != channels_.end() && iter->second == channel;
}

void Epoller::assertInLoopThread() const {
  owner_loop_->assertInLoopThread();
}

void Epoller::fillActiveChannels(ChannelList *activeChannels, int numEvents) const {
  assert(numEvents <= static_cast<int>(events_.size()));
  for (int i = 0; i < numEvents; ++i) {
    auto channel = static_cast<Channel *>(events_[i].data.ptr);
    channel->set_revents(events_[i].events);
    activeChannels->push_back(channel);
  }
}

void Epoller::update(Channel *channel, int operation) {
  struct epoll_event event{};
  event.events = channel->events();
  event.data.ptr = channel;
  const int fd = channel->fd();
  LOG_DEBUG << "epoll_ctl op = " << operationToString(operation)
            << " fd = " << fd << " event = { " << channel->eventsToString() << " }";

  if (::epoll_ctl(epoll_fd_, operation, fd, &event) < 0) {
    LOG_DEBUG << "epoll_ctl op = " << operationToString(operation)
              << " fd = " << fd;
  }
}

const char *Epoller::operationToString(int op) {
  if (op == EPOLL_CTL_ADD) return "ADD";
  else if (op == EPOLL_CTL_MOD) return "MOD";
  else if (op == EPOLL_CTL_DEL) return "DEL";
  return "Unknown Operation";
}