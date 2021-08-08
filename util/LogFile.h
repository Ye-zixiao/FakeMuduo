//
// Created by Ye-zixiao on 2021/8/7.
//

#ifndef LIBFM_UTIL_LOGFILE_H_
#define LIBFM_UTIL_LOGFILE_H_

#include <string>
#include <memory>
#include <fstream>
#include <unistd.h>

namespace fm::log {

class LogLine;

class LogFile {
 public:
  LogFile(const std::string &log_directory,
          const std::string &log_file_name,
          uint32_t roll_file_size_max,
          uint32_t check_lines = 1000,
          uint32_t flush_interval = 3);

  void write(LogLine &log_line);

 private:
  std::string getLogFileName(time_t now);
  void rollFile();

 private:
  static constexpr uint32_t kRollPerSeconds = 60 * 60 * 24;

  const std::string log_file_name_;
  const uint32_t log_file_max_size_;// 最大日志写入量（bytes）
  const uint32_t check_lines_;      // 检查行间隔
  const time_t flush_interval_;     // 刷新间隔，默认3s
  std::streamoff writen_bytes_;     // 已写入量（bytes）
  uint32_t line_counts_;            // 当前轮写入行数，用于辅助上述检查行

  std::unique_ptr<std::ofstream> log_file_;
  time_t start_log_time_;           // 开始日志时间，也是滚动日志时间
  time_t last_flush_time_;          // 最后一次刷新时间
};

} // namespace fm::log

#endif //LIBFM_UTIL_LOGFILE_H_
