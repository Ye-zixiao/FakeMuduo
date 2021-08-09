//
// Created by Ye-zixiao on 2021/8/7.
//

#include "libfm/fmutil/LogLine.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include "libfm/fmutil/TimeStamp.h"
#include "libfm/fmutil/SystemClock.h"

namespace {

//// 实际上异步日志的时候根本就不需要保证线程权限，不过对于StdLogger这样的直接
//// 输出到标准输出的日志器而言，strerror()最基本的线程安全性是必要的
//thread_local char t_error_buf[512];
//
//inline const char *strerrorThreadSafe(int saved_errno) {
//  return ::strerror_r(saved_errno, t_error_buf, sizeof(t_error_buf));
//}

pid_t thisThreadId() {
  static thread_local pid_t this_thread_id = gettid();
  return this_thread_id;
}

} // unnamed namespace

namespace fm::log {

enum class SupportedTypes : uint8_t {
  kChar = 0,
  kUInt32,
  kUInt64,
  kInt32,
  kInt64,
  kDouble,
  kStrView,  // 在LogLine内部保存一个string_view，对于那些字符串常量值非常节省空间
  kRawStr    // 保存一个string，此时需要将所有的字符串保存在LogLine内部的缓冲区中
};

const char *logLevelToString(LogLevel log_level) {
  switch (log_level) {
    case LogLevel::kINFO:
      return "INFO";
    case LogLevel::kWARN:
      return "WARN";
    case LogLevel::kERROR:
      return "ERROR";
    case LogLevel::kDEBUG:
      return "DEBUG";
    case LogLevel::kFATAL:
      return "FATAL";
    case LogLevel::kCLOSE:
      return "CLOSE";
  }
  return "UNKNOWN";
}

char *LogLine::buffer() {
  return heap_buffer_ ? &heap_buffer_[used_bytes_] : &stack_buffer_[used_bytes_];
}

template<typename T>
void LogLine::encode(T t) {
  *reinterpret_cast<T *>(buffer()) = t;
  this->used_bytes_ += sizeof(T);
}

template<typename T>
void LogLine::encode(SupportedTypes type, T t) {
  resizeBufferIfNeeded(sizeof(uint8_t) + sizeof(T));
  encode<SupportedTypes>(type);
  encode<T>(t);
}

void LogLine::encodeStrView(std::string_view sv) {
  encode<std::string_view>(SupportedTypes::kStrView, sv);
}

void LogLine::encodeString(std::string_view sv) {
  if (sv.length() == 0) return;

  resizeBufferIfNeeded(1 + sv.size() + 1);
  char *b = buffer();
  *reinterpret_cast<uint8_t *>(b++) = static_cast<uint8_t>(SupportedTypes::kRawStr);
  memcpy(b, sv.data(), sv.size() + 1);
  used_bytes_ += 1 + sv.size() + 1;
}

template<typename T>
void LogLine::decode(std::ostream &os, char *&b) {
  T t = *reinterpret_cast<T *>(b);
  os << t;
  b += sizeof(T);
}

void LogLine::decodeStrView(std::ostream &os, char *&b) {
  std::string_view sv = *reinterpret_cast<std::string_view *>(b);
  os << sv;
  b += sizeof(std::string_view);
}

void LogLine::decodeRawStr(std::ostream &os, char *&b) {
  std::string_view sv(b);
  os << sv;
  b += sv.size() + 1;
}

LogLine::LogLine(LogLevel level, int old_errno, const char *file, const char *func, uint32_t line)
    : used_bytes_(0),
      buffer_size_(sizeof(stack_buffer_)),
      heap_buffer_(),
      stack_buffer_{} {
  static_assert(sizeof(LogLine) == 128, "sizeof LogLine != 128");
  encode<time::TimeStamp>(time::SystemClock::now());
  encode<pid_t>(thisThreadId());
  encode<LogLevel>(level);
  encode<std::string_view>(file);
  encode<std::string_view>(func);
  encode<uint32_t>(line);
  encode<int>(old_errno);
}

void LogLine::stringify(std::ostream &os) {
  char *b = heap_buffer_ ? heap_buffer_.get() : stack_buffer_;
  const char *const end = b + used_bytes_;

  time::TimeStamp timestamp = *reinterpret_cast<time::TimeStamp *>(b);
  b += sizeof(time::TimeStamp);
  pid_t thread_id = *reinterpret_cast<pid_t *>(b);
  b += sizeof(pid_t);
  LogLevel log_level = *reinterpret_cast<LogLevel *>(b);
  b += sizeof(LogLevel);
  std::string_view file = *reinterpret_cast<std::string_view *>(b);
  b += sizeof(std::string_view);
  std::string_view func = *reinterpret_cast<std::string_view *>(b);
  b += sizeof(std::string_view);
  uint32_t line = *reinterpret_cast<uint32_t *>(b);
  b += sizeof(uint32_t);
  int old_errno = *reinterpret_cast<int *>(b);
  b += sizeof(int);

  // 日志格式：
  // 2021-07-27 17:40:46.980012 27177 [INFO] "func_name" (errno message) hello - Logger_test.cpp:25
  // 编码方式：
  //       int64_t              pid_t uint8_t  string_view string_view uint32_t 后续的部分按照类型码+数据的组合进行编码
  // 但这种引入实时解析的方法可能会一定程度上降低日志输出的性能
  os << timestamp.toString(true) << ' ' << thread_id;
  os << " [" << logLevelToString(log_level) << "] <" << func << "> ";
  if (old_errno != 0) {
    os << "(" << strerror(old_errno) << ") ";
    errno = 0;
  }
  stringify(os, b, end);
  // 不要使用std::endl来输出换行符，因为它会造成一次非必要的刷新操作
  os << " - " << basename(file.data()) << ':' << line << '\n';

  if (log_level == LogLevel::kFATAL) {
    os.flush();
    abort();
  }
}

void LogLine::resizeBufferIfNeeded(size_t needed_bytes) {
  const size_t required_size = used_bytes_ + needed_bytes;
  if (required_size <= buffer_size_) return;

  if (!heap_buffer_) {
    buffer_size_ = std::max(static_cast<size_t>(512), required_size);
    heap_buffer_.reset(new char[buffer_size_]);
    ::memcpy(heap_buffer_.get(), stack_buffer_, used_bytes_);
  } else {
    buffer_size_ = std::max(static_cast<size_t>(2 * buffer_size_), required_size);
    std::unique_ptr<char[]> new_heap_buffer(new char[buffer_size_]);
    ::memcpy(new_heap_buffer.get(), heap_buffer_.get(), used_bytes_);
    heap_buffer_ = std::move(new_heap_buffer);
  }
}

LogLine &LogLine::operator<<(char c) {
  encode<char>(SupportedTypes::kChar, c);
  return *this;
}

LogLine &LogLine::operator<<(int32_t i32) {
  encode<int32_t>(SupportedTypes::kInt32, i32);
  return *this;
}

LogLine &LogLine::operator<<(int64_t i64) {
  encode<int64_t>(SupportedTypes::kInt64, i64);
  return *this;
}

LogLine &LogLine::operator<<(uint32_t u32) {
  encode<uint32_t>(SupportedTypes::kUInt32, u32);
  return *this;
}

LogLine &LogLine::operator<<(uint64_t u64) {
  encode<uint64_t>(SupportedTypes::kUInt64, u64);
  return *this;
}

LogLine &LogLine::operator<<(double d) {
  encode<double>(SupportedTypes::kDouble, d);
  return *this;
}

LogLine &LogLine::operator<<(const std::string &str) {
  encodeString(str.c_str());
  return *this;
}

LogLine &LogLine::operator<<(std::string_view sv) {
  encodeStrView(sv);
  return *this;
}

LogLine &LogLine::operator<<(const void *vp) {
  encode<uint64_t>(SupportedTypes::kUInt64, reinterpret_cast<uint64_t>(vp));
  return *this;
}

void LogLine::stringify(std::ostream &os, char *b, const char *const end) {
  while (b < end) {
    auto type = *reinterpret_cast<SupportedTypes *>(b++);
    switch (type) {
      case SupportedTypes::kChar:
        decode<char>(os, b);
        break;
      case SupportedTypes::kInt32:
        decode<int32_t>(os, b);
        break;
      case SupportedTypes::kInt64:
        decode<int64_t>(os, b);
        break;
      case SupportedTypes::kUInt32:
        decode<uint32_t>(os, b);
        break;
      case SupportedTypes::kUInt64:
        decode<uint64_t>(os, b);
        break;
      case SupportedTypes::kDouble:
        decode<double>(os, b);
        break;
      case SupportedTypes::kStrView:
        decodeStrView(os, b);
        break;
      case SupportedTypes::kRawStr:
        decodeRawStr(os, b);
        break;
      default:
        return;
    }
  }
}

} // namespace fm::log