//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef LIBFM_BASE_EXCEPTION_H_
#define LIBFM_BASE_EXCEPTION_H_

#include <exception>
#include <string>

namespace fm {

// 一个能够记录调用过程中的函数栈帧信息的异常
class Exception : public std::exception {
 public:
  explicit Exception(std::string what);
  ~Exception() noexcept override = default;

  const char *what() const noexcept override {
    return message_.c_str();
  }

  const char *stackTrace() const noexcept {
    return stack_.c_str();
  }

 private:
  std::string message_;
  std::string stack_;
};

} // namespace fm

#endif //LIBFM_BASE_EXCEPTION_H_
