//
// Created by Ye-zixiao on 2021/8/7.
//

#include <string>
#include <thread>
#include "libfm/fmutil/Log.h"
using namespace std;

int main() {
  constexpr uint32_t mb = 1024 * 1024;
  fm::log::initialize(fm::log::AsyncLoggerTag{}, "./",
                      "async_logger", 32 * mb);
  fm::log::setLogLevel(fm::log::LogLevel::kINFO);

  thread t1([] {
    for (int i = 0; i < 10000; ++i)
      LOG_DEBUG << "talk is cheap, show me the code" << std::string(", hello earth");
  });
  thread t2([] {
    for (int i = 0; i < 10000; ++i)
      LOG_ERROR << "talk is cheap, show me the code" << std::string(", hello earth");
  });
  thread t3([] {
    for (int i = 0; i < 10000; ++i)
      LOG_WARN << "talk is cheap, show me the code" << std::string(", hello earth");
  });
  for (int i = 0; i < 10000; ++i)
    LOG_INFO << "hello world" << std::string(", hello china") << std::string("a string");

  t1.join();
  t2.join();
  t3.join();
  return 0;
}