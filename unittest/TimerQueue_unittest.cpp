//
// Created by Ye-zixiao on 2021/5/5.
//

#include <unistd.h>
#include <iostream>

#include "libfm/base.h"
#include "libfm/net.h"

using namespace std;
using namespace fm;
using namespace fm::net;

void printInfo() {
  cout << "pid " << ::getpid() << ", tid " << ::gettid()
       << " says: hello world at " << time::SystemClock::now().toString()
       << endl;
}

int main() {
//  Logger::setLogLevel(Logger::TRACE);

  EventLoop event_loop;
  cout << "start time: " << time::SystemClock::now().toString() << endl;
  auto timerId = event_loop.runEvery(3s, printInfo);
  event_loop.runAfter(5s, printInfo);
  event_loop.runAfter(10s, [timerId, &event_loop] {
    event_loop.cancel(timerId);
    cout << "cancel timer(" << timerId.sequence() << ")"
         << " at " << time::SystemClock::now().toString() << endl;
    event_loop.quit();
  });
  event_loop.loop();
  return 0;
}