//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef LIBFM_BASE_NONCOAPYABLE_H_
#define LIBFM_BASE_NONCOAPYABLE_H_

namespace fm {

class NonCopyable {
 public:
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable &operator=(const NonCopyable &) = delete;

 protected:
  NonCopyable() = default;
  ~NonCopyable() = default;
};

} // namespace fm

#endif //LIBFM_BASE_NONCOAPYABLE_H_
