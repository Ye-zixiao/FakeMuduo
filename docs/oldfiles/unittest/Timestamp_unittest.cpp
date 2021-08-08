//
// Created by Ye-zixiao on 2021/6/6.
//

#include <unordered_set>
#include <iostream>
#include <string>
#include <set>

#include "libfm/base.h"

using namespace std;
using namespace fm;
using namespace fm::time;

int main() {
  Logger::setLogLevel(Logger::TRACE);
  LOG_INFO << "Timestamp unit tests start";

  Timestamp now(SystemClock::now());
  cout << "now: " << now.toString(true) << endl;

  Timestamp yesterday = now - 24h;
  cout << "yesterday: " << yesterday.toString() << endl;

  Timestamp tomorrow = now + 24h;
  cout << "tomorrow: " << tomorrow.toFormattedString("%Y-%m-%d %H:%M:%S") << endl;

  Timestamp oldTime = now - 60min;
  Timestamp newTime = now + 30min;
  if (oldTime < newTime) {
    cout << "oldTime: " << oldTime.toString() << endl;
    cout << "newTime: " << newTime.toString() << endl;
  }

  set<Timestamp> timeSet;
  timeSet.insert(now);
  timeSet.insert(yesterday);
  timeSet.insert(oldTime);
  timeSet.insert(newTime);
  timeSet.insert(tomorrow);
  cout << "In TimeSet: " << endl;
  for (const auto &time:timeSet)
    cout << time.toString(true) << endl;

  unordered_set<Timestamp> unorderedTimeSet;
  unorderedTimeSet.insert(now);
  unorderedTimeSet.insert(yesterday);
  unorderedTimeSet.insert(newTime);
  unorderedTimeSet.insert(oldTime);
  unorderedTimeSet.insert(tomorrow);
  cout << "In UnorderedTimeSet: " << endl;
  for (const auto &time:unorderedTimeSet)
    cout << time.toString(true) << endl;

  auto spec = time::SystemClock::to_timespec(time::Timestamp(3s));
  cout << spec.tv_sec << ' ' << spec.tv_nsec << endl;

  LOG_INFO << "Timestamp unit tests end";

  return 0;
}

