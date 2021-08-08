//
// Created by Ye-zixiao on 2021/8/7.
//

#ifndef LIBFM_INCLUDE_LIBFM_FMLOG_LOGLINE_H_
#define LIBFM_INCLUDE_LIBFM_FMLOG_LOGLINE_H_

#include <string_view>
#include <memory>

namespace fm::log {

enum class LogLevel : uint8_t {
  kINFO = 0, kWARN = 1, kERROR = 2, kDEBUG = 3, kFATAL = 4
};

enum class SupportedTypes : uint8_t;

class LogLine {
 public:
  // 构造一个无效的日志行
  LogLine() : used_bytes_(0),
              buffer_size_(sizeof(stack_buffer_)),
              heap_buffer_(), stack_buffer_{} {}

  LogLine(LogLevel level, int old_errno, const char *file,
          const char *func, uint32_t line);
  ~LogLine() = default;

  // 可移动但不可拷贝
  LogLine(LogLine &&) noexcept = default;
  LogLine &operator=(LogLine &&) noexcept = default;

  void stringify(std::ostream &os);

  LogLine &operator<<(char c);
  LogLine &operator<<(int32_t i32);
  LogLine &operator<<(uint32_t u32);
  LogLine &operator<<(int64_t i64);
  LogLine &operator<<(uint64_t u64);
  LogLine &operator<<(double d);
  LogLine &operator<<(const std::string &str);
  LogLine &operator<<(std::string_view sv);
  LogLine &operator<<(const void *vp);

  template<size_t N>
  LogLine &operator<<(const char (&cstr)[N]) {
    encodeStrView(std::string_view(cstr));
    return *this;
  }

  template<typename T>
  std::enable_if_t<std::is_same<T, const char *>::value, LogLine &>
  operator<<(const T &cstr) {
    encodeStrView(std::string_view(cstr));
    return *this;
  }

  template<typename T>
  std::enable_if_t<std::is_same<T, char *>::value, LogLine &>
  operator<<(const T &cstr) {
    encodeStrView(std::string_view(cstr));
    return *this;
  }

 private:
  char *buffer();
  void resizeBufferIfNeeded(size_t needed_bytes);
  void stringify(std::ostream &os, char *start, const char *const end);

 private:
  template<typename T>
  void encode(T t);

  template<typename T>
  void encode(SupportedTypes type, T t);

  void encodeStrView(std::string_view sv);
  void encodeString(std::string_view sv);

  template<typename T>
  void decode(std::ostream &os, char *&b);

  void decodeStrView(std::ostream &os, char *&b);
  void decodeRawStr(std::ostream &os, char *&b);

 private:
  static constexpr size_t kLogLineSize = 128;

  size_t used_bytes_;
  size_t buffer_size_;
  std::unique_ptr<char[]> heap_buffer_;
  char stack_buffer_[kLogLineSize - sizeof(size_t) * 2 - sizeof(decltype(heap_buffer_))];
};

} // namespace fm::log

#endif //LIBFM_INCLUDE_LIBFM_FMLOG_LOGLINE_H_
