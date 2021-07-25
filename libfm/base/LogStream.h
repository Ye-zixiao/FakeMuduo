//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef LIBFM_BASE_LOGSTREAM_H_
#define LIBFM_BASE_LOGSTREAM_H_

#include <cstring>
#include <string>
#include "libfm/base/NonCopyable.h"

namespace fm {

template<int SIZE>
class FixedBuffer : private NonCopyable {
 public:
  FixedBuffer() : data_{}, curr_(data_) {}

  void append(const char *buf, int len) {
    if (avail() > len) {
      memcpy(curr_, buf, len);
      curr_ += len;
    }
  }

  void reset() { curr_ = data_; }
  void bzero() { memset(data_, 0, sizeof(data_)); }

  const char *data() const { return data_; }
  char *current() { return curr_; }

  int length() const { return static_cast<int>(curr_ - data_); }
  int avail() const { return static_cast<int>(end() - curr_); }

  std::string toString() const { return std::string(data_, length()); }

 private:
  const char *end() const { return data_ + sizeof(data_); }

  char data_[SIZE];
  char *curr_;
};

class LogStream : private NonCopyable {
 public:
  static constexpr int kSmallBuffer = 4096;
  static constexpr int kLargeBuffer = 4096 * 1024;

  using Buffer = FixedBuffer<kSmallBuffer>;
  using self = LogStream;

  void append(const char *str, int len) { buffer_.append(str, len); }

  // fix it: 是否可以完全用模板的方式替代？
  self &operator<<(short s) { return numericConvertToStr(s); }
  self &operator<<(unsigned short us) { return numericConvertToStr(us); }
  self &operator<<(int i) { return numericConvertToStr(i); }
  self &operator<<(unsigned int ui) { return numericConvertToStr(ui); }
  self &operator<<(long l) { return numericConvertToStr(l); }
  self &operator<<(unsigned long ul) { return numericConvertToStr(ul); }
  self &operator<<(long long ll) { return numericConvertToStr(ll); }
  self &operator<<(float f) { return *this << static_cast<double>(f); }
  self &operator<<(double d) { return numericConvertToStr(d); }

  self &operator<<(bool v) {
    buffer_.append(v ? "1" : "0", 1);
    return *this;
  }

  self &operator<<(char c) {
    buffer_.append(&c, 1);
    return *this;
  }

  self &operator<<(const char *str) {
    if (str) buffer_.append(str, strlen(str));
    else buffer_.append("(null)", 6);
    return *this;
  }

  self &operator<<(const void *ptr) {
    char buf[32]{};
    auto uintptr = reinterpret_cast<uintptr_t>(ptr);
    snprintf(buf, sizeof(buf), "0x%lx", uintptr);
    buffer_.append(buf, ::strlen(buf));
    return *this;
  }

  self &operator<<(std::string_view str) {
    buffer_.append(str.data(), str.size());
    return *this;
  }

  const Buffer &buffer() const { return buffer_; }
  void resetBuffer() { buffer_.reset(); }

 private:
  template<typename T>
  self &numericConvertToStr(T t) {
    std::string str(std::to_string(t));
    buffer_.append(str.c_str(), str.length());
    return *this;
  }

 private:
  Buffer buffer_;
};

} // namespace fm

#endif //LIBFM_BASE_LOGSTREAM_H_
