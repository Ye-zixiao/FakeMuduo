//
// Created by Ye-zixiao on 2021/8/7.
//

#include <atomic>
#include <thread>
#include <string>
#include <iostream>
#include <csignal>
#include "util/LogBufferQueue.h"
using namespace std;
using namespace fm::log;

std::atomic<bool> g_flag(false);
std::atomic<bool> g_run(true);

void signal_handler(int signo) {
  g_run.store(false, std::memory_order_relaxed);
}

int main() {
  LogBufferQueue log_buffer_queue{};

  if (signal(SIGINT, signal_handler) == SIG_ERR)
    return -1;

  thread t1([&log_buffer_queue] {
    LogLine log_line{};
    int count = 0;
    while (!g_flag.load(std::memory_order_acquire));
    while (g_run.load(std::memory_order_relaxed)) {
      if (log_buffer_queue.pop(log_line)) {
        log_line.stringify(cout);

        if (++count == 10000) {
          // 每输出1w行就看下缓冲区的使用情况
          int new_time, reuse_time, backup_push_time, backup_pop_time, backup_size;
          getAllocationForDebug(new_time, reuse_time, backup_push_time,
                                backup_pop_time, backup_size);
          cerr << "new time: " << new_time << ", reuse time: " << reuse_time
               << ", backup push time: " << backup_push_time << ", backup pop time: "
               << backup_pop_time << ", backup size: " << backup_size << endl;
          count = 0;
        }
      }
    }
    while (log_buffer_queue.pop(log_line))
      log_line.stringify(cout);
  });
  g_flag.store(true, std::memory_order_release);
  thread t2([&log_buffer_queue] {
    while (g_run.load(std::memory_order_relaxed)) {
      this_thread::sleep_for(10us);
      LogLine log_line(fm::log::LogLevel::kERROR, 2, __FILE__, __func__, __LINE__);
      log_line << "hello world, " << std::string_view("show me the code");
      log_buffer_queue.push(std::move(log_line));
    }
  });
  while (g_run.load(std::memory_order_relaxed)) {
    this_thread::sleep_for(10us);
    LogLine log_line(fm::log::LogLevel::kDEBUG, 1, __FILE__, __func__, __LINE__);
    log_line << "pi: " << 3.1425926 << ", root(3): " << 1.7320508075688;
    log_buffer_queue.push(std::move(log_line));
  }

  t1.join();
  t2.join();
  cerr << "\ntest end" << endl;

  return 0;
}
