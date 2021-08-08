//
// Created by Ye-zixiao on 2021/8/7.
//

#include "util/AsyncLogger.h"
#include <chrono>
#include "libfm/fmutil/LogLine.h"
#include "util/LogFile.h"
#include "util/LogBufferQueue.h"

namespace fm::log {

AsyncLogger::AsyncLogger(const std::string &log_directory,
                         const std::string &log_file_name,
                         uint32_t roll_file_size_max)
    : state_(STATE::kINIT),
      buffer_queue_(std::make_unique<LogBufferQueue>()),
      log_file_(std::make_unique<LogFile>(log_directory, log_file_name, roll_file_size_max)),
      background_thread_(&AsyncLogger::popThread, this) {
  state_.store(STATE::kREADY, std::memory_order_release);
}

AsyncLogger::~AsyncLogger() {
  state_.store(STATE::kSHUTDOWN, std::memory_order_relaxed);
  background_thread_.join();
}

void AsyncLogger::add(LogLine &&log_line) {
  buffer_queue_->push(std::move(log_line));
}

void AsyncLogger::popThread() {
  while (state_.load(std::memory_order_acquire) == STATE::kINIT)
    std::this_thread::sleep_for(std::chrono::microseconds(50));

  // 正式开始处理日志的输出
  LogLine log_line;
  while (state_.load(std::memory_order_relaxed) == STATE::kREADY) {
    if (buffer_queue_->pop(log_line))
      log_file_->write(log_line);
    else
      std::this_thread::sleep_for(std::chrono::microseconds(50));
  }

  // 处理剩下的未完成的日志输出
  while (buffer_queue_->pop(log_line))
    log_file_->write(log_line);
}

} // namespace fm::log