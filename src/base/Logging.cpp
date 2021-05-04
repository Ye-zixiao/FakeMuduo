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

/***
 * 以线程安全的方式返回错误编号errno
 * @param savedErrno
 * @return
 */
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

// 设置日志消息提示级别
Logger::LogLevel g_logLevel = initLogLevel();

const char *LogLevelName[Logger::NUM_LOG_LEVELS]{
	"[TRACE] ", "[DEBUG] ", "[INFO] ",
	"[WARN] ", "[ERROR] ", "[FATAL] "
};

inline LogStream &operator<<(LogStream &ls, const Logger::SourceFile &sf) {
  ls.append(sf.data_, sf.size_);
  return ls;
}

void defaultOutput(const char *msg, int len) {
  fwrite(msg, 1, len, stdout); // why fwrite？
}

void defaultFlush() {
  fflush(stdout);
}

// 默认情况下，日志消息是输出到标准输出中的
// 这两个函数主要在下面的~Logger()中使用
Logger::OutputFunc g_output = defaultOutput;
Logger::FlushFunc g_flush = defaultFlush;

} // namespace fm

using namespace fm;

Logger::Impl::Impl(LogLevel level, int old_errno, const SourceFile &file, int line)
	: time_(TimeStamp::now()),
	  stream_(),
	  level_(level),
	  line_(line),
	  basename_(file) {
  formatTime();
  stream_ << ' ' << gettid() << ' '; // 不使用pthread_self()
  stream_ << LogLevelName[level];
  if (old_errno != 0)
	stream_ << strerror_ts(old_errno) << " (errno=" << old_errno << ") ";
}

void Logger::Impl::formatTime() {
  stream_ << time_.toFormattedString();
}

void Logger::Impl::finish() {
  stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(SourceFile file, int line)
	: impl_(INFO, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level)
	: impl_(level, 0, file, line) {}

Logger::Logger(SourceFile file, int line, LogLevel level, const char *func)
	: impl_(level, 0, file, line) {
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
  g_output(buf.data(), buf.length());
  if (impl_.level_ == FATAL) {
	g_flush();
	abort();
  }
}

void Logger::setOutput(OutputFunc output_func) {
  g_output = output_func;
}

void Logger::setFlush(FlushFunc flush_func) {
  g_flush = flush_func;
}

void Logger::setLogLevel(LogLevel level) {
  g_logLevel = level;
}