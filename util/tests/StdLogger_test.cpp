//
// Created by Ye-zixiao on 2021/8/7.
//

#include <thread>
#include <iostream>
#include "libfm/fmutil/Log.h"
using namespace std;

int main() {
  auto start = fm::time::SystemClock::now();
  fm::log::initialize(fm::log::StdLoggerTag{});

  thread t1([] {
    for (int i = 0; i < 10000; ++i)
      LOG_INFO << "hello world, " << "show me the code";
  });
  thread t2([] {
    for (int i = 0; i < 10000; ++i)
      LOG_INFO << "hello world, " << "show me the code";
  });
  thread t3([] {
    for (int i = 0; i < 10000; ++i)
      LOG_ERROR << "pi: " << 3.1425926 << ", root(3): " << 1.7320508075688;
  });
  thread t4([] {
    for (int i = 0; i < 10000; ++i)
      LOG_ERROR << "pi: " << 3.1425926 << ", root(3): " << 1.7320508075688;
  });
  t1.join();
  t2.join();
  t3.join();
  t4.join();
  auto end = fm::time::SystemClock::now();
  cerr << (end - start).count() << endl;
  return 0;
}