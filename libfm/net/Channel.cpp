//
// Created by Ye-zixiao on 2021/4/7.
//

#include "libfm/net/Channel.h"
#include <sstream>
#include <sys/poll.h>
#include <cassert>
#include "libfm/base/Logging.h"
#include "libfm/net/EventLoop.h"

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
      log_hup_(false),
      tied_(false),
      event_handling_(false),
      added_to_loop_(false) {}

Channel::~Channel() {
  assert(!event_handling_);
  assert(!added_to_loop_);
  if (loop_->isInLoopThread())
    assert(!loop_->hasChannel(this));
}

void Channel::tie(const std::shared_ptr<void> &obj) {
  // 只有当有一个TcpConnection与当前频道关联的时候才会使用
  // tie()这个函数，此时Channel就可以弱引用TcpConnection
  tie_ = obj;
  tied_ = true;
}

void Channel::update() {
  added_to_loop_ = true;
  loop_->updateChannel(this);
}

void Channel::remove() {
  assert(isNoneEvent());
  added_to_loop_ = false;
  loop_->removeChannel(this);
}

void Channel::handleEvent(time::Timestamp receiveTime) {
  std::shared_ptr<void> guard;
  if (tied_) {
    // 防止在执行回调函数的过程中将通道的所有者TcpServer杀死！
    guard = tie_.lock();
    if (guard)
      handleEventWithGurad(receiveTime);
    LOG_TRACE << "guard use_count: " << guard.use_count(); // 3
  } else {
    handleEventWithGurad(receiveTime);
  }
}

void Channel::handleEventWithGurad(time::Timestamp receiveTime) {
  // 通道的操纵权在事件通知时属于I/O线程，所以不需要加锁
  event_handling_ = true;
  LOG_TRACE << reventsToString();

  // 处理3种异常事件
  if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
    if (log_hup_)
      LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
    if (close_callback_) close_callback_();
  }
  if (revents_ & POLLNVAL)
    LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLINVAL";
  if (revents_ & (POLLERR | POLLNVAL))
    if (error_callback_) error_callback_();

  // 处理正常事件
  if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    if (read_callback_) read_callback_(receiveTime);
  if (revents_ & POLLOUT)
    if (write_callback_) write_callback_();

  event_handling_ = false;
}

std::string Channel::eventsToString(int fd, int ev) {
  char buf[32]{};
  int pr = snprintf(buf, 32, "%d: ", fd);
  if (ev & POLLIN)
    snprintf(buf + pr, 32 - pr, "IN ");
  if (ev & POLLPRI)
    snprintf(buf + pr, 32 - pr, "PRI ");
  if (ev & POLLOUT)
    snprintf(buf + pr, 32 - pr, "OUT ");
  if (ev & POLLHUP)
    snprintf(buf + pr, 32 - pr, "HUP ");
  if (ev & POLLRDHUP)
    snprintf(buf + pr, 32 - pr, "RDHUP ");
  if (ev & POLLERR)
    snprintf(buf + pr, 32 - pr, "ERR ");
  if (ev & POLLNVAL)
    snprintf(buf + pr, 32 - pr, "NVAL ");
  return buf;
}