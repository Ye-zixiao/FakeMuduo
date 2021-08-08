//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef LIBFM_NET_CHANNEL_H_
#define LIBFM_NET_CHANNEL_H_

#include <functional>
#include <memory>

namespace fm {

namespace time {
class TimeStamp;
} // namespace fm::time

namespace net {

class EventLoop;

class Channel : private NonCopyable {
 public:
  // 可读事件的回调函数比较特殊，需要向用户的回调函数传递一个接收时间信息
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(time::TimeStamp)>;

  Channel(EventLoop *loop, int fd);

  ~Channel();

  void handleEvent(time::TimeStamp receiveTime);

  void setReadCallback(ReadEventCallback cb) { read_callback_ = std::move(cb); }
  void setWriteCallback(EventCallback cb) { write_callback_ = std::move(cb); }
  void setCloseCallback(EventCallback cb) { close_callback_ = std::move(cb); }
  void setErrorCallback(EventCallback cb) { error_callback_ = std::move(cb); }

  void enableReading() { events_ |= kReadEvent, update(); }
  void enableWriting() { events_ |= kWriteEvent, update(); }
  void disableReading() { events_ &= ~kReadEvent, update(); }
  void disableWriting() { events_ &= ~kWriteEvent, update(); }
  void disableAll() { events_ = kNoneEvent, update(); }
  bool isReading() const { return events_ & kReadEvent; }
  bool isWriting() const { return events_ & kWriteEvent; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }

  std::string eventsToString() const { return eventsToString(fd_, events_); }
  std::string reventsToString() const { return eventsToString(fd_, revents_); }

  void set_index(int idx) { index_ = idx; }
  int index() const { return index_; }

  void set_revents(int revents) { revents_ = revents; }
  int events() const { return events_; }

  int fd() const { return fd_; }

  EventLoop *ownerLoop() { return loop_; }

  void tie(const std::shared_ptr<void> &);

  void remove();

 private:
  void update();

  void handleEventWithGurad(time::TimeStamp receiveTime);

  static std::string eventsToString(int fd, int ev);

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

 private:
  EventLoop *loop_;
  const int fd_;
  int events_;
  int revents_;
  int index_;
  bool log_hup_;          // 连接挂掉的时候是否需要进行日志

  std::weak_ptr<void> tie_;
  bool tied_;            // 是否与某一个连接绑定
  bool event_handling_;   // 是否正在处理事件
  bool added_to_loop_;     // 是否添加到事件循环中
  ReadEventCallback read_callback_;
  EventCallback write_callback_;
  EventCallback close_callback_;
  EventCallback error_callback_;
};

} // namespace fm::net

} // namespace fm

#endif //LIBFM_NET_CHANNEL_H_
