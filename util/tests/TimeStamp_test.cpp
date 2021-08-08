//
// Created by Ye-zixiao on 2021/8/7.
//

#include <iostream>
#include <string>
#include <cstring>
#include "libfm/fmutil/TimeStamp.h"
#include "libfm/fmutil/SystemClock.h"
using namespace std;
using namespace fm::time;

int main() {
  auto timestamp = SystemClock::now();
  cout << timestamp.toString() << endl;
  cout << timestamp.toString(true) << endl;
  cout << (timestamp + 2s).toString(true) << endl;
  cout << (timestamp + 50ms).toString(true) << endl;
  cout << (timestamp + 1h).toString(true) << endl;
  cout << (timestamp + 24h).toString(true) << endl;
  cout << timestamp.toFormattedString("%Y-%m-%d") << endl;

  decltype(timestamp) epoch_timestamp = SystemClock::zero();
  cout << epoch_timestamp.toString(true) << endl;

  auto random_time = TimeStamp(1h);
  cout << random_time.toFormattedString("%F %T") << endl;

  cout << sizeof(TimeStamp) << endl;

  char buf[8];
  memcpy(buf, &timestamp, sizeof(timestamp));

  auto saved_timestamp = *reinterpret_cast<TimeStamp *>(buf);
  cout << saved_timestamp.toString() << endl;

  cout << saved_timestamp.toString(true) << " " << 12324 << " [INFO] "
  << basename(__FILE__) << ":" << __LINE__ << ":" << __FUNCTION__
  << " - " << "hello world" << endl;

  return 0;
}