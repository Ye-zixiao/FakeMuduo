//
// Created by Ye-zixiao on 2021/8/7.
//

#include "libfm/fmutil/Log.h"
#include <memory>
#include <cassert>
#include "util/BaseLogger.h"
#include "util/StdLogger.h"
#include "util/AsyncLogger.h"

namespace fm::log {

std::unique_ptr<BaseLogger> logger;
std::atomic<BaseLogger *> atomic_logger{};

void initialize(StdLoggerTag) {
  logger = std::make_unique<StdLogger>();
  atomic_logger.store(logger.get(), std::memory_order_release);
}

void initialize(AsyncLoggerTag,
                const std::string &log_directory,
                const std::string &log_file_name,
                uint32_t log_file_size_max) {
  logger = std::make_unique<AsyncLogger>(log_directory, log_file_name, log_file_size_max);
  atomic_logger.store(logger.get(), std::memory_order_release);
}

std::atomic<LogLevel> atomic_log_level{LogLevel::kINFO};

LogLevel currentLogLevel() {
  return atomic_log_level.load(std::memory_order_relaxed);
}

void setLogLevel(LogLevel log_level) {
  atomic_log_level.store(log_level, std::memory_order_release);
}

bool isLowerThanOrEqualToCurr(LogLevel log_level) {
  return log_level <= atomic_log_level.load(std::memory_order_acquire);
}

bool FmLog::operator==(LogLine &log_line) {
  assert(atomic_logger.load(std::memory_order_relaxed));
  atomic_logger.load(std::memory_order_acquire)->add(std::move(log_line));
  return true;
}

} // namespace fm::log