//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef FAKEMUDUO_BASE_LOGGING_H_
#define FAKEMUDUO_BASE_LOGGING_H_

#include "TimeStamp.h"
#include "LogStream.h"

namespace fm {

class Logger {
 public:
  // 内部类SourceFile用以记录__FILE__这样的源文件字符串
  // 信息，不过我们只会使用该文件的basename，其他的不要
  class SourceFile {
   public:
    template<int N>
    SourceFile(const char (&arr)[N])
        :data_(arr), size_(N - 1) {
      const char *slash = strrchr(data_, '/');
      if (slash) {
        data_ = slash + 1;
        size_ -= static_cast<int>(data_ - arr);
      }
    }

    explicit SourceFile(const char *filename)
        : data_(filename) {
      const char *slash = strrchr(filename, '/');
      if (slash)
        data_ = slash + 1;
      size_ = static_cast<int>(strlen(data_));
    }

    const char *data_;
    int size_;
  };

 public:
  enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL, NUM_LOG_LEVELS };

  using OutputFunc = void (*)(const char *msg, int len);
  using FlushFunc = void (*)();

  static void setOutput(OutputFunc outputFunc) { outputFunc_ = outputFunc; }
  static void setFlush(FlushFunc flushFunc) { flushFunc_ = flushFunc; }
  static void setLogLevel(LogLevel logLevel) { logLevel_ = logLevel; }
  static LogLevel logLevel() { return logLevel_; }

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char *func);
  Logger(SourceFile file, int line, bool toAbort);
  ~Logger();

  LogStream &stream() { return impl_.stream_; }

 private:
  // 日志内部实现类，负责日志信息的收集和格式化
  class Impl {
   public:
    using LogLevel = Logger::LogLevel;

    Impl(LogLevel level, int old_errno, const SourceFile &file, int line);
    void formatTime();
    void finish();

    TimeStamp time_;
    LogStream stream_;
    LogLevel level_;
    int line_;
    SourceFile basename_;
  };

 private:
  static LogLevel logLevel_;
  static OutputFunc outputFunc_;
  static FlushFunc flushFunc_;

  Impl impl_;
};

// 把这个接口留给用户有什么用？
const char *strerror_ts(int savedErrno);

#define LOG_TRACE if(Logger::logLevel() <= Logger::TRACE) \
    Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG if(Logger::logLevel() <= Logger::DEBUG) \
    Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()

#define LOG_INFO if (Logger::logLevel() <= Logger::INFO) \
    Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::FATAL).stream()
#define LOG_SYSERR Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL Logger(__FILE__, __LINE__, true).stream()

} // namespace fm

#endif //FAKEMUDUO_BASE_LOGGING_H_