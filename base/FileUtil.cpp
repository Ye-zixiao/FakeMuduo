//
// Created by Ye-zixiao on 2021/4/7.
//

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cassert>
#include <algorithm>

#include "FileUtil.h"
#include "Logging.h"

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
      buf_{} {
  if (fd_ < 0) err_ = errno;
}

ReadSmallFile::~ReadSmallFile() {
  if (fd_ >= 0) ::close(fd_);
}

/**
 * 读取指定文件中的数据，并将这些数据放入到给定的string中
 * @param maxSize    给定string可容纳的最大数据量
 * @param content    文件数据需要存放到的指定位置
 * @param fileSize   获取文件大小
 * @param modifyTime 获取文件最近修改时间
 * @param createTime 获取文件创建时间
 * @return           错误编码
 */
int ReadSmallFile::readToString(int maxSize, std::string *content, int64_t *fileSize,
                                int64_t *modifyTime, int64_t *createTime) {
  assert(content);
  int err = err_;
  if (fd_ >= 0) {
    content->clear();

    if (fileSize) {
      struct stat statBuf{};
      if (::fstat(fd_, &statBuf) == 0) {
        if (S_ISREG(statBuf.st_mode)) {
          *fileSize = statBuf.st_size;
          content->reserve(static_cast<size_t>(std::min(static_cast<int64_t>(maxSize), *fileSize)));
        } else if (S_ISDIR(statBuf.st_mode)) {
          err = EISDIR;
        }
        if (modifyTime) *modifyTime = statBuf.st_mtim.tv_sec;
        if (createTime) *createTime = statBuf.st_ctim.tv_sec;
      } else {
        err = errno;
      }
    }

    while (content->size() < static_cast<size_t>(maxSize)) {
      // 将文件中的数据在不超出给定string最大大小的限定下，存放到content指定的string中
      size_t toRead = std::min(static_cast<size_t>(maxSize) - content->size(), sizeof(buf_));
      ssize_t nRead = ::read(fd_, buf_, toRead);
      if (nRead > 0)
        content->append(buf_, nRead);
      else {
        if (nRead < 0) err = errno;
        break;
      }
    }
  }
  return err;
}

int ReadSmallFile::readToBuffer(int *size) {
  int err = err_;
  if (fd_ >= 0) {
    ssize_t nRead = ::pread(fd_, buf_, sizeof(buf_) - 1, 0);
    if (nRead >= 0) {
      if (size) *size = static_cast<int>(nRead);
      buf_[nRead] = '\0';
    } else {
      err = errno;
    }
  }
  return err;
}