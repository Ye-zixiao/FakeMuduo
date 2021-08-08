//
// Created by Ye-zixiao on 2021/5/5.
//

#include "libfm/base.h"

using namespace fm;

int main() {
  Logger::setLogLevel(Logger::TRACE);
  LOG_TRACE << "hello world" << ", show me the code";
  LOG_INFO << "talk is cheap";
  return 0;
}
