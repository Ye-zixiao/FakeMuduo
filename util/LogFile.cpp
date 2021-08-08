//
// Created by Ye-zixiao on 2021/8/7.
//

#include "util/LogFile.h"
#include "libfm/fmutil/LogLine.h"

namespace {

inline std::string_view thisHostName() {
  static char buf[32]{};
  gethostname(buf, sizeof(buf));
  return buf;
}

inline std::string_view timeToString(time_t now) {
  static char timebuf[32]{};
  static struct tm tmbuf{};
  localtime_r(&now, &tmbuf);
  strftime(timebuf, sizeof(timebuf), ".%Y%m%d-%H%M%S.", &tmbuf);
  return timebuf; // 字符串之后没必要担心后面‘\0’的问题
}

} // unnamed namespace

namespace fm::log {

LogFile::LogFile(const std::string &log_directory,
                 const std::string &log_file_name,
                 uint32_t roll_file_size_max,
                 uint32_t check_lines,
                 uint32_t flush_interval)
    : log_file_name_(log_directory + log_file_name),
      log_file_max_size_(roll_file_size_max),
      check_lines_(check_lines),
      flush_interval_(flush_interval),
      writen_bytes_(0),
      line_counts_(0),
      log_file_(),
      start_log_time_(0),
      last_flush_time_(0) {
  rollFile();
}

void LogFile::write(LogLine &log_line) {
  auto pos = log_file_->tellp();
  log_line.stringify(*log_file_);
  writen_bytes_ += log_file_->tellp() - pos;

  if (writen_bytes_ >= log_file_max_size_) {
    // 超过指定的最大日志文件大小
    rollFile();
  } else {
    ++line_counts_;
    // 每个一段数量的日志行就检查一次
    if (line_counts_ >= check_lines_) {
      line_counts_ = 0;
      time_t now(time(nullptr));
      time_t current_day = now / kRollPerSeconds * kRollPerSeconds;

      if (current_day != start_log_time_)
        // 若到了新的一天
        rollFile();
      else if (now - last_flush_time_ > flush_interval_) {
        // 离上一次刷新时间有了一定时间间隔，则自动刷新一下
        last_flush_time_ = now;
        log_file_->flush();
      }
    }
  }
}

// 其实这种直接使用fstrtime、time原始接口的方式应该被淘汰
std::string LogFile::getLogFileName(time_t now) {
  std::string new_log_file_name;
  new_log_file_name.reserve(log_file_name_.size() + 64);
  new_log_file_name = log_file_name_;

  // 获取时间、主机名
  new_log_file_name.append(timeToString(now));
  new_log_file_name.append(thisHostName());
  new_log_file_name.append(".log");

  return new_log_file_name;
}

void LogFile::rollFile() {
  if (log_file_) {
    log_file_->flush();
    log_file_->close();
  }

  time_t now = ::time(nullptr);
  std::string new_log_file_name = getLogFileName(now);
  time_t start_log_time = now / kRollPerSeconds * kRollPerSeconds;
  start_log_time_ = start_log_time;
  last_flush_time_ = now;

  writen_bytes_ = 0;
  line_counts_ = 0;
  log_file_ = std::make_unique<std::ofstream>();
  log_file_->open(new_log_file_name, std::ofstream::out | std::ofstream::trunc);
}

} // namespace fm::log