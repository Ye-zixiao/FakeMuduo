//
// Created by Ye-zixiao on 2021/4/7.
//

#include "libfm/base/LogFile.h"

#include <unistd.h>

using namespace fm;

LogFile::LogFile(std::string basename,
                 off_t rollFileSizeMax,
                 bool threadSafe,
                 int flushInterval,
                 int logLineMax)
    : basename_(std::move(basename)),
      rollFileSizeMax_(rollFileSizeMax),
      flushInterval_(flushInterval),
      checkOfNLines(logLineMax),
      mutex_(threadSafe ? new std::mutex : nullptr),
      startOfLog_(0),
      lastRoll_(0),
      lastFlush_(0),
      count_(0) {
  // 执行一次日志文件滚动更新，因为刚开始时日志文件还没有创建
  rollFile();
}

void LogFile::append(const char *logline, size_t len) {
  if (mutex_) {
    std::lock_guard<std::mutex> lock(*mutex_);
    append_unlocked(logline, len);
  } else {
    append_unlocked(logline, len);
  }
}

void LogFile::flush() {
  if (mutex_) {
    std::lock_guard<std::mutex> lock(*mutex_);
    file_->flush();
  } else {
    file_->flush();
  }
}

void LogFile::append_unlocked(const char *logline, size_t len) {
  file_->append(logline, len);

  if (file_->writenBytes() > rollFileSizeMax_) {
    rollFile();
  } else {
    ++count_;
    if (count_ >= checkOfNLines) {
      count_ = 0;
      time_t now = ::time(nullptr);
      time_t currentPeriod = now / kRollPerSeconds_ * kRollPerSeconds_;
      if (currentPeriod != startOfLog_) {
        rollFile();
      } else if (now - lastFlush_ > flushInterval_) {
        lastFlush_ = now;
        file_->flush();
      }
    }
  }
}

void LogFile::rollFile() {
  time_t now = 0;
  std::string filename = getLogFileName(basename_, &now);
  time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

  lastRoll_ = now;
  lastFlush_ = now;
  startOfLog_ = start;
  file_ = std::make_unique<AppendFile>(filename);
}

std::string LogFile::getLogFileName(const std::string &basename, time_t *now) {
  std::string filename;
  filename.reserve(basename.size() + 64);
  filename = basename;

  char timebuf[32];
  struct tm tmbuf{};
  *now = ::time(nullptr);
  localtime_r(now, &tmbuf);
  // 这里有一个比较有意思的地方就是，若日志滚动的太快，快到一秒钟之内滚动了多次，
  // 那么LogFile就会重复打开同一个日志文件！！不过这种只会在测试时设置很小的文件
  // 大小限制和很小的运行工作量的时候才会出现这个问题，实际中日志文件会设置的很大，
  // 同时服务器的工作也总是长时间工作、工作量大，因此不会出现这种问题！
  strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", &tmbuf);
  filename.append(timebuf);

  char pidbuf[32];
  gethostname(pidbuf, sizeof(pidbuf));
  filename.append(pidbuf);
  filename.append(".log");
  return filename;
}