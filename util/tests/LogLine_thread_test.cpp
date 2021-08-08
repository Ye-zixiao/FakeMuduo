//
// Created by Ye-zixiao on 2021/8/7.
//

#include <thread>
#include <iostream>
#include "libfm/fmutil/LogLine.h"
#include "util/SpinLock.h"
using namespace std;

std::atomic_flag g_flag{};

int main() {
  thread t1([] {
    for (int i = 0; i < 10000; ++i) {
      fm::log::LogLine log_line(fm::log::LogLevel::kERROR, 0, __FILE__, __func__, __LINE__);
      log_line << "hello world, " << "show me the code";
      fm::log::SpinLock spin_lock(g_flag);
      log_line.stringify(cout);
    }
  });
  thread t2([] {
    for (int i = 0; i < 10000; ++i) {
      fm::log::LogLine log_line(fm::log::LogLevel::kERROR, 0, __FILE__, __func__, __LINE__);
      log_line << "hello world, " << "show me the code";
      fm::log::SpinLock spin_lock(g_flag);
      log_line.stringify(cout);
    }
  });
  thread t3([] {
    for (int i = 0; i < 10000; ++i) {
      fm::log::LogLine log_line(fm::log::LogLevel::kERROR, 0, __FILE__, __func__, __LINE__);
      log_line << "hello world, " << "show me the code";
      fm::log::SpinLock spin_lock(g_flag);
      log_line.stringify(cout);
    }
  });
  for (int i = 0; i < 10000; ++i) {
    fm::log::LogLine log_line(fm::log::LogLevel::kERROR, 0, __FILE__, __func__, __LINE__);
    log_line << "hello world, " << "show me the code";
    fm::log::SpinLock spin_lock(g_flag);
    log_line.stringify(cout);
  }
  t1.join();
  t2.join();
  t3.join();
  return 0;
}