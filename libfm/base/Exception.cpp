//
// Created by Ye-zixiao on 2021/4/7.
//

#include "libfm/base/Exception.h"

#include <execinfo.h>

using namespace std;
using namespace fm;

string stackTrace() {
  string stack;
  const int max_frames = 200;
  void *frame[max_frames];

  int nptrs = ::backtrace(frame, max_frames);
  char **strings = ::backtrace_symbols(frame, nptrs);
  if (strings) {
    for (int i = 0; i < nptrs; ++i) {
      stack.append(strings[i]);
      stack.push_back('\n');
    }
  }
  free(strings);
  return stack;
}

Exception::Exception(std::string what)
    : message_(std::move(what)), stack_(::stackTrace()) {}