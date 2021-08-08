//
// Created by Ye-zixiao on 2021/8/7.
//

#ifndef LIBFM_INCLUDE_LIBFM_FMLOG_LOG_H_
#define LIBFM_INCLUDE_LIBFM_FMLOG_LOG_H_

#include <string>
#include "libfm/fmutil/LogLine.h"
#include "libfm/fmutil/TimeStamp.h"
#include "libfm/fmutil/SystemClock.h"

namespace fm::log {

struct StdLoggerTag {};
struct AsyncLoggerTag {};

void initialize(StdLoggerTag);
void initialize(AsyncLoggerTag,
                const std::string &log_directory,
                const std::string &log_file_name,
                uint32_t log_file_size_max);

LogLevel currentLogLevel();
void setLogLevel(LogLevel log_level);
bool isLowerThanOrEqualToCurr(LogLevel log_level);

struct FmLog {
  bool operator==(LogLine &log_line);
};

} // namespace fm::log

#define LOG(LEVEL) fm::log::FmLog() == fm::log::LogLine(LEVEL, errno, __FILE__, __func__, __LINE__)
#define LOG_FATAL isLowerThanOrEqualToCurr(fm::log::LogLevel::kFATAL) && LOG(fm::log::LogLevel::kFATAL)
#define LOG_DEBUG isLowerThanOrEqualToCurr(fm::log::LogLevel::kDEBUG) && LOG(fm::log::LogLevel::kDEBUG)
#define LOG_ERROR isLowerThanOrEqualToCurr(fm::log::LogLevel::kERROR) && LOG(fm::log::LogLevel::kERROR)
#define LOG_WARN isLowerThanOrEqualToCurr(fm::log::LogLevel::kWARN) && LOG(fm::log::LogLevel::kWARN)
#define LOG_INFO isLowerThanOrEqualToCurr(fm::log::LogLevel::kINFO) && LOG(fm::log::LogLevel::kINFO)

#endif //LIBFM_INCLUDE_LIBFM_FMLOG_LOG_H_
