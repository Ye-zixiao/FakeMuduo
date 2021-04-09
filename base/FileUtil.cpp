//
// Created by Ye-zixiao on 2021/4/7.
//

#include "FileUtil.h"
#include "Logging.h"

using namespace fm;

AppendFile::AppendFile(const std::string &filename)
	: fp_(::fopen(filename.c_str(), "a")), writenBytes_(0) {
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

  writenBytes_ += len;
}

void AppendFile::flush() {
  ::fflush(fp_);
}

size_t AppendFile::write(const char *logline, size_t len) {
  return ::fwrite_unlocked(logline, 1, len, fp_);
}