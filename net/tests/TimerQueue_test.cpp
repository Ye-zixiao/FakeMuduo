//
// Created by Ye-zixiao on 2021/8/8.
//

#include <iostream>
#include <unistd.h>
#include "libfm/fmutil/SystemClock.h"
#include "libfm/fmutil/TimeStamp.h"
#include "libfm/fmutil/Log.h"
#include "libfm/fmnet/EventLoop.h"
#include "libfm/fmnet/TimerId.h"

using namespace std;
using namespace fm::net;

void printInfo() {
  cout << "pid " << ::getpid() << ", tid " << ::gettid()
       << " says: hello world at " << fm::time::SystemClock::now().toString()
       << endl;
}

int main() {
  fm::log::setLogLevel(fm::log::LogLevel::kINFO);
  fm::log::initialize(fm::log::StdLoggerTag{});

  EventLoop loop;
  cout << "start time: " << fm::time::SystemClock::now().toString() << endl;

  auto timer_id = loop.runEvery(3s, printInfo);
  loop.runAfter(5s, printInfo);
  loop.runAfter(10s, [timer_id, &loop] {
    loop.cancel(timer_id);
    cout << "cancel timer(" << timer_id.sequence() << ")"
         << " at " << fm::time::SystemClock::now().toString()
         << endl;
    loop.quit();
  });

  loop.loop();
  return 0;
}