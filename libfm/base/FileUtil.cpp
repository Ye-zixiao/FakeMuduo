//
// Created by Ye-zixiao on 2021/4/7.
//

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cassert>
#include <algorithm>

#include "libfm/base/FileUtil.h"
#include "libfm/base/Logging.h"

using namespace fm;

AppendFile::AppendFile(const std::string &filename)
    : fp_(::fopen(filename.c_str(), "a")),
      writenBytes_(0),
      buffer_{} {
  // 重置文件流的用户写缓冲区
  ::setbuffer(fp_, buffer_, sizeof(buffer_));
}

AppendFile::~AppendFile() {
  ::fclose(fp_);
}

void AppendFile::append(const char *logline, size_t len) {
  size_t n = write(logline, len);
  size_t remain = len - n;
  while (remain > 0) {
    size_t x = write(logline, remain);
    if (x == 0) {
      int error = ferror(fp_);
      if (error)
        fprintf(stderr, "AppendFile::append() failed: %s\n", strerror_ts(errno));
      break;
    }
    remain -= x;
  }

  writenBytes_ += static_cast<off_t>(len);
}

void AppendFile::flush() {
  ::fflush(fp_);
}

size_t AppendFile::write(const char *logline, size_t len) {
  return ::fwrite_unlocked(logline, 1, len, fp_);
}

ReadSmallFile::ReadSmallFile(const std::string &filename)
    : fd_(::open(filename.c_str(), O_RDONLY | O_CLOEXEC)),
      err_(0),
      fileSize_(0),
      mtime_(),
      ctime_() {
  if (fd_ < 0) err_ = errno;

  // 记录文件元信息
  struct stat statBuf{};
  if (::fstat(fd_, &statBuf) == 0) {
    if (S_ISREG(statBuf.st_mode)) {
      fileSize_ = statBuf.st_size;
    } else if (S_ISDIR(statBuf.st_mode)) {
      err_ = errno;
    }
    mtime_ = time::SystemClock::from_timespec(statBuf.st_mtim);
    ctime_ = time::SystemClock::from_timespec(statBuf.st_ctim);
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
    size_t contentSize = std::min(maxContentSize, fileSize_);
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
                            time::Timestamp *mtime,
                            time::Timestamp *ctime) {
  ReadSmallFile file(filename);
  if (fileSize) *fileSize = file.fileSize();
  if (mtime) *mtime = file.mtime();
  if (ctime) *ctime = file.ctime();
  return file.readToString(content, maxContentSize);
}