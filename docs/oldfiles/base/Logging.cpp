//
// Created by Ye-zixiao on 2021/4/7.
//

#include "Logging.h"
#include <cerrno>
#include <cstring>
#include <cstdio>
#include <unistd.h>

namespace fm {

thread_local char t_errnoBuf[512];
thread_local char t_time[64];
thread_local time_t t_lastSecond;

// 以线程安全的方式返回错误编号errno
const char *strerror_ts(int savedErrno) {
  return strerror_r(savedErrno, t_errnoBuf, sizeof(t_errnoBuf));
}

Logger::LogLevel initLogLevel() {
  if (::getenv("LOG_TRACE"))
    return Logger::TRACE;
  else if (::getenv("LOG_DEBUG"))
    return Logger::DEBUG;
  return Logger::INFO;
}

const char *LogLevelName[Logger::NUM_LOG_LEVELS]{
    "[TRACE] ", "[DEBUG] ", "[INFO] ",
    "[WARN] ", "[ERROR] ", "[FATAL] "
};

inline LogStream &operator<<(LogStream &ls, const Logger::SourceFile &sf) {
  ls.append(sf.data_, sf.size_);
  return ls;
}

void defaultOutput(const char *msg, int len) {
  fwrite(msg, 1, len, stdout); // why fwrite?
}

void defaultFlush() {
  fflush(stdout);
}

} // namespace fm

using namespace fm;

Logger::LogLevel Logger::log_level_ = initLogLevel();
Logger::OutputFunc Logger::output_func_ = defaultOutput;
Logger::FlushFunc Logger::flush_func_ = defaultFlush;

Logger::Impl::Impl(LogLevel level, int old_errno, const SourceFile &file, int line)
    : time_(time::SystemClock::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file) {
  formatTime();
  stream_ << ' ' << gettid() << ' '; // 不使用pthread_self()
  stream_ << LogLevelName[level];
  if (old_errno != 0) {
    stream_ << strerror_ts(old_errno) << " (errno = " << old_errno << ") ";
    errno = 0;
  }
}

void Logger::Impl::formatTime() {
  stream_ << time_.toString(true);
}

void Logger::Impl::finish() {
  stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line)
    : impl_(INFO, errno, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level)
    : impl_(level, errno, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char *func)
    : impl_(level, errno, file, line) {
  impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, bool toAbort) :
    impl_(toAbort ? FATAL : ERROR, errno, file, line) {}

/***
 * 当Logger类析构的时候自动将缓冲区保存的格式化日志消息输出到指定的输出流中，
 * 输出后，日志格式类似于如下形式：
 *  当前时间 [日志级别] 用户自定义信息 - 当前源文件名:行数
 */
Logger::~Logger() {
  impl_.finish();
  const LogStream::Buffer &buf(stream().buffer());
  output_func_(buf.data(), buf.length());
  if (impl_.level_ == FATAL) {
    flush_func_();
    abort();
  }
}