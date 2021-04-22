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
  /***
   * 内部类SourceFile用以记录__FILE__源文件信息，
   * 但只会记录最后的basename！
   */
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

  static void setOutput(OutputFunc);
  static void setFlush(FlushFunc);
  static void setLogLevel(LogLevel);
  static LogLevel logLevel();

  Logger(SourceFile file, int line);
  Logger(SourceFile file, int line, LogLevel level);
  Logger(SourceFile file, int line, LogLevel level, const char *func);
  Logger(SourceFile file, int line, bool toAbort);
  ~Logger();

  LogStream &stream() { return impl_.stream_; }

 private:
  /***
   * 日志内部实现类，负责日志信息的收集和格式化
   */
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

  Impl impl_;
};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel() { return g_logLevel; }

const char *strerror_ts(int savedErrno);

// lOG_TRACE和LOG_DEBUG会默认在前面加上函数名__func__
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
