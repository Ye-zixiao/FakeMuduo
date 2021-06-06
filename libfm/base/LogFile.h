//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef LIBFM_BASE_LOGFILE_H_
#define LIBFM_BASE_LOGFILE_H_

#include <ctime>
#include <memory>
#include <mutex>

#include "libfm/base/noncoapyable.h"
#include "libfm/base/FileUtil.h"

namespace fm {

class LogFile : private noncopyable {
 public:
  LogFile(std::string filename,
		  off_t rollFileSizeMax,
		  bool threadSafe = true,
		  int flushInterval = 3,
		  int logLineMax = 1024);
  ~LogFile() = default;

  void append(const char *logline, size_t len);
  void flush();
  void rollFile(); // 不再返回bool，它应该总是成功的

 private:
  void append_unlocked(const char *logline, size_t len);

  static std::string getLogFileName(const std::string &basename, time_t *now);

 private:
  static constexpr int kRollPerSeconds_ = 60 * 60 * 24;

  const std::string basename_;
  const off_t rollFileSizeMax_;   // 滚动日志文件写入字节数上限
  const int flushInterval_;       // 刷新间隔
  const int checkOfNLines;        // 每输出logLineMax_行之后就检查一下时间（是否到了新的一天）

  std::unique_ptr<std::mutex> mutex_;
  std::unique_ptr<AppendFile> file_;
  time_t startOfLog_;             // 当前日志文件开始记录的时间
  time_t lastRoll_;               // 上一次滚动日志的时间
  time_t lastFlush_;              // 上一次刷新缓冲区的时间
  int count_;                     // 记录当前离上一轮检查后输出的日志消息行数
};

} // namespace fm

#endif //LIBFM_BASE_LOGFILE_H_
