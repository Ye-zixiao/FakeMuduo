//
// Created by Ye-zixiao on 2021/5/5.
//

#include "../../src/FakeMuduo.h"

#include <unistd.h>
#include <iostream>

using namespace std;
using namespace fm;
using namespace fm::net;

void printInfo() {
  cout << "pid: " << ::getpid() << ", tid: " << ::gettid() << endl;
  cout << "currTime: " << TimeStamp::now().toFormattedString() << endl;
}

int main() {
//  Logger::setLogLevel(Logger::TRACE);

  EventLoop event_loop;
  cout << "start time: " << TimeStamp::now().toFormattedString() << endl;
  auto timerId = event_loop.runEvery(3, printInfo);
  event_loop.runAfter(5, printInfo);
  event_loop.runAfter(10, [timerId, &event_loop] {
    event_loop.cancel(timerId);
    cout << "cancel timer(" << timerId.sequence() << ")"
         << " at " << TimeStamp::now().toFormattedString() << endl;
    event_loop.quit();
  });
  event_loop.loop();
  return 0;
}