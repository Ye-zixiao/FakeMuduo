//
// Created by Ye-zixiao on 2021/4/8.
//

#include "EpollPoller.h"

#include <sys/epoll.h>
#include <cassert>
#include <cerrno>
#include <unistd.h>

#include "../../base/Logging.h"
#include "../Channel.h"

using namespace fm;
using namespace fm::net;

namespace {
constexpr int kNew = -1;
constexpr int kAdded = 1;
constexpr int kDeleted = 2;
}

EpollPoller::EpollPoller(EventLoop *loop)
	: Poller(loop),
	  epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
	  events_(kInitEventListSize) {
  if (epollfd_ < 0)
	LOG_SYSFATAL << "EpollPoller::EpollPoller";
}

EpollPoller::~EpollPoller() {
  ::close(epollfd_);
}

TimeStamp EpollPoller::poll(ChannelList *activeChannels, int timeoutMs) {
  LOG_TRACE << "fd total count " << channels_.size();
  int numEvents = ::epoll_wait(epollfd_,
							   events_.data(),
							   static_cast<int>(events_.size()),
							   timeoutMs);
  int savedErrno = errno;
  TimeStamp curr(TimeStamp::now());
  if (numEvents > 0) {
	LOG_TRACE << numEvents << " events happened";
	fillActiveChannels(activeChannels, numEvents);
	if (numEvents == static_cast<int>(events_.size()))
	  events_.resize(events_.size() * 2);
  } else if (numEvents == 0) {
	LOG_TRACE << "nothing happened";
  } else {
	if (savedErrno != EINTR) {
	  errno = savedErrno;
	  LOG_SYSERR << "EpollPoller::poll";
	}
  }
  return curr;
}

void EpollPoller::updateChannel(Channel *channel) {
  Poller::assertInLoopThread();

  // 对于EpollPoller而言，这个index表示通道的状态
  const int index = channel->index();
  const int fd = channel->fd();
  LOG_TRACE << "fd = " << fd << " events = {" << channel->eventsToString()
			<< "} index = " << index;

  if (index == kNew || index == kDeleted) {
	if (index == kNew) {
	  assert(channels_.find(fd) == channels_.end());
	  channels_[fd] = channel;
	} else {
	  // 原来就应该存在
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
	} else
	  update(channel, EPOLL_CTL_MOD);
  }
}

void EpollPoller::removeChannel(Channel *channel) {
  Poller::assertInLoopThread();

  const int fd = channel->fd();
  const int index = channel->index();
  LOG_TRACE << "fd = " << fd;
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  assert(index == kAdded || index == kDeleted);
  size_t n = channels_.erase(fd);
  assert(n == 1);

  if (index == kAdded)
	update(channel, EPOLL_CTL_DEL);
  channel->set_index(kNew);
}

void EpollPoller::fillActiveChannels(ChannelList *activeChannels, int numEvents) const {
  assert(numEvents <= static_cast<int>(events_.size()));
  for (int i = 0; i < numEvents; ++i) {
	auto *channel = static_cast<Channel *>(events_[i].data.ptr);
	channel->set_revents(events_[i].events);
	activeChannels->push_back(channel);
  }
}

void EpollPoller::update(Channel *channel, int operation) {
  struct epoll_event event{};
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();
  LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
			<< " fd = " << fd << " event = { " << channel->eventsToString() << " }";

  // 完成真正的向内核epoll实例注册/修改/删除相关文件描述符关心事件
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
	if (operation == EPOLL_CTL_DEL)
	  LOG_SYSERR << "epoll_ctl op = " << operationToString(operation)
				 << " fd = " << fd;
	else
	  LOG_SYSERR << "epoll_ctl op = " << operationToString(operation)
				 << " fd = " << fd;
  }
}

const char *EpollPoller::operationToString(int op) {
  if (op == EPOLL_CTL_ADD) return "ADD";
  else if (op == EPOLL_CTL_MOD) return "MOD";
  else if (op == EPOLL_CTL_DEL) return "DEL";
  return "Unknown Operation";
}