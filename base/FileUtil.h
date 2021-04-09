//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef FAKEMUDUO_BASE_FILEUTIL_H_
#define FAKEMUDUO_BASE_FILEUTIL_H_

#include "noncoapyable.h"

#include <cstdio>
#include <string>

namespace fm {

class AppendFile : private noncopyable {
 public:
  explicit AppendFile(const std::string &filename);
  ~AppendFile();

  void append(const char *logline, size_t len);

  void flush();

  off_t writenBytes() const { return writenBytes_; }

 private:
  size_t write(const char *logline, size_t len);

  FILE *fp_;
  char buffer_[64 * 1024];
  off_t writenBytes_;
};

} // namespace fm

#endif //FAKEMUDUO_BASE_FILEUTIL_H_
