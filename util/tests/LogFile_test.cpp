//
// Created by Ye-zixiao on 2021/8/7.
//

#include "util/LogFile.h"
#include "libfm/fmutil/LogLine.h"
using namespace std;
using namespace fm::log;

int main() {
  LogLine log_line(LogLevel::kINFO, 0, __FILE__, __func__, __LINE__);
  log_line << "hello world" << INT32_MAX << ", show me the code";

  constexpr uint32_t mb = 1024 * 1024;
  LogFile log_file("./", "LogFile_test", 32 * mb);
  for (int i = 0; i < 10000; ++i)
    log_file.write(log_line);

  return 0;
}