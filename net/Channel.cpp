//
// Created by Ye-zixiao on 2021/4/7.
//

#include "Channel.h"

#include <sstream>
#include <sys/poll.h>
#include <cassert>

#include "../base/Logging.h"
#include "EventLoop.h"

using namespace fm;
using namespace fm::net;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop *loop, int fd)
	: loop_(loop),
	  fd_(fd),
	  events_(0),
	  revents_(0),
	  index_(-1),
	  logHup_(false),
	  tied_(false),
	  eventHandling_(false),
	  addedToLoop_(false) {}

Channel::~Channel() {
  assert(!eventHandling_);
  assert(!addedToLoop_);
  if (loop_->isInLoopThread())
	assert(!loop_->hasChannel(this));
}

void Channel::tie(const std::shared_ptr<void> &obj) {
  // 表示和某一个连接或者定时器或者事件通知器进行绑定了，
  // 因而也表示此时的通道是有效的
  tie_ = obj;
  tied_ = true;
}

void Channel::update() {
  addedToLoop_ = true;
  loop_->updateChannel(this);
}

void Channel::remove() {
  assert(isNoneEvent());
  addedToLoop_ = false;
  loop_->removeChannel(this);
}

void Channel::handleEvent(TimeStamp receiveTime) {
  std::shared_ptr<void> guard;
  if (tied_) {
	// 防止在执行回调函数的过程中将通道的所有者TcpServer杀死！
	guard = tie_.lock();
	if (guard)
	  handleEventWithGurad(receiveTime);
  } else {
	handleEventWithGurad(receiveTime);
  }
}

void Channel::handleEventWithGurad(TimeStamp receiveTime) {
  // 通道的操纵权在事件通知时属于I/O线程，所以不需要加锁
  eventHandling_ = true;
  LOG_TRACE << reventsToString();

  // 处理3种异常事件
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
	if (logHup_)
	  LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
	if (closeCallback_) closeCallback_();
  }
  if (revents_ & POLLNVAL)
	LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLINVAL";
  if (revents_ & (POLLERR | POLLNVAL))
	if (errorCallback_) errorCallback_();

  // 处理正常事件
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
	if (readCallback_) readCallback_(receiveTime);
  if (revents_ & POLLOUT)
	if (writeCallback_) writeCallback_();

  eventHandling_ = false;
}

std::string Channel::eventsToString(int fd, int ev) {
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & POLLIN)
	oss << "IN ";
  if (ev & POLLPRI)
	oss << "PRI ";
  if (ev & POLLOUT)
	oss << "OUT ";
  if (ev & POLLHUP)
	oss << "HUP ";
  if (ev & POLLRDHUP)
	oss << "RDHUP ";
  if (ev & POLLERR)
	oss << "ERR ";
  if (ev & POLLNVAL)
	oss << "NVAL ";
  return oss.str();
}