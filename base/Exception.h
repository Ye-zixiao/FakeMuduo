//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef FAKEMUDUO_BASE_EXCEPTION_H_
#define FAKEMUDUO_BASE_EXCEPTION_H_

#include <exception>
#include <string>

namespace fm {

// 一个能够显示记录抛出异常调用过程下函数栈帧信息
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

} //namespace fm

#endif //FAKEMUDUO_BASE_EXCEPTION_H_
