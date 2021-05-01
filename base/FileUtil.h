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
  off_t writenBytes_;
  char buffer_[64 * 1024];
};

// 用来读取小文件，不过实际到底想要读取大多的文件的权力掌握在
// readSmallFile()函数的maxSize函数的手里
class ReadSmallFile : private noncopyable {
 public:
  static constexpr int kBufferSize = 128 * 1024;

  explicit ReadSmallFile(const std::string &filename);
  ~ReadSmallFile();

  int readToString(int maxSize,
                   std::string *content,
                   int64_t *fileSize,
                   int64_t *modifyTime,
                   int64_t *createTime);

  int readToBuffer(int *size);

  const char *buffer() const { return buf_; }

 private:
  int fd_;
  int err_;
  char buf_[kBufferSize];
};

inline
int readSmallFile(int maxSize,
                  const std::string &filename,
                  std::string *content,
                  int64_t *fileSize = nullptr,
                  int64_t *modifyTime = nullptr,
                  int64_t *createTime = nullptr) {
  ReadSmallFile file(filename);
  return file.readToString(maxSize, content, fileSize, modifyTime, createTime);
}

} // namespace fm

#endif //FAKEMUDUO_BASE_FILEUTIL_H_
