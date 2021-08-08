//
// Created by Ye-zixiao on 2021/8/8.
//

#include "ReadSmallFile.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cassert>
#include <algorithm>
#include "libfm/fmutil/SystemClock.h"

using namespace examples;

ReadSmallFile::ReadSmallFile(const std::string &filename)
    : fd_(::open(filename.c_str(), O_RDONLY | O_CLOEXEC)),
      err_(0),
      file_size_(0),
      mtime_(),
      ctime_() {
  if (fd_ < 0) err_ = errno;

  // 记录文件元信息
  struct stat statBuf{};
  if (::fstat(fd_, &statBuf) == 0) {
    if (S_ISREG(statBuf.st_mode)) {
      file_size_ = statBuf.st_size;
    } else if (S_ISDIR(statBuf.st_mode)) {
      err_ = errno;
    }
    mtime_ = fm::time::SystemClock::from_timespec(statBuf.st_mtim);
    ctime_ = fm::time::SystemClock::from_timespec(statBuf.st_ctim);
  } else {
    err_ = errno;
  }
}

ReadSmallFile::~ReadSmallFile() {
  if (fd_ >= 0) ::close(fd_);
}

int ReadSmallFile::readToString(std::string *content, size_t maxContentSize) {
  assert(content);
  int err = err_;

  if (fd_ >= 0) {
    size_t contentSize = std::min(maxContentSize, file_size_);
    content->resize(contentSize);
    size_t readStart = 0;

    while (readStart < contentSize) {
      size_t needToRead = contentSize - readStart;
      ssize_t nRead = ::read(fd_, content->data() + readStart, needToRead);
      if (nRead > 0)
        readStart += nRead;
      else {
        if (nRead < 0) err = errno;
        break;
      }
    }
  }
  return err;
}

int ReadSmallFile::readFile(const std::string &filename,
                            std::string *content,
                            size_t maxContentSize,
                            size_t *fileSize,
                            fm::time::TimeStamp *mtime,
                            fm::time::TimeStamp *ctime) {
  ReadSmallFile file(filename);
  if (fileSize) *fileSize = file.fileSize();
  if (mtime) *mtime = file.mtime();
  if (ctime) *ctime = file.ctime();
  return file.readToString(content, maxContentSize);
}