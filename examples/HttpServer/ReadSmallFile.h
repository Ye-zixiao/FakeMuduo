//
// Created by Ye-zixiao on 2021/8/8.
//

#ifndef OLD_LIBFM_EXAMPLE_HTTPSERVER_READSMALLFILE_H_
#define OLD_LIBFM_EXAMPLE_HTTPSERVER_READSMALLFILE_H_

#include <string>
#include <libfm/fmutil/TimeStamp.h>

namespace examples {

class ReadSmallFile {
 public:
  explicit ReadSmallFile(const std::string &filename);
  ~ReadSmallFile();

  ReadSmallFile(const ReadSmallFile &) = delete;
  ReadSmallFile &operator=(const ReadSmallFile &) = delete;

  size_t fileSize() const { return file_size_; }

  const fm::time::TimeStamp &mtime() const { return mtime_; }
  const fm::time::TimeStamp &ctime() const { return ctime_; }

  int readToString(std::string *content, size_t maxContentSize);

 public:
  static constexpr size_t k128KB = 1024 * 128;
  static constexpr size_t k512KB = 1024 * 512;
  static constexpr size_t k1MB = 1024 * 1024;
  static constexpr size_t k64MB = 1024 * 1024 * 64;

  static int readFile(const std::string &filename,
                      std::string *content,
                      size_t maxContentSize,
                      size_t *fileSize = nullptr,
                      fm::time::TimeStamp *mtime = nullptr,
                      fm::time::TimeStamp *ctime = nullptr);

 private:
  int fd_;
  int err_;
  size_t file_size_;
  fm::time::TimeStamp mtime_;
  fm::time::TimeStamp ctime_;
};

} // namespace examples

#endif //OLD_LIBFM_EXAMPLE_HTTPSERVER_READSMALLFILE_H_
