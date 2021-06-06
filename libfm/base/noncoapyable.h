//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef LIBFM_BASE_NONCOAPYABLE_H_
#define LIBFM_BASE_NONCOAPYABLE_H_

namespace fm {

class noncopyable {
 public:
  noncopyable(const noncopyable &) = delete;
  noncopyable &operator=(const noncopyable &) = delete;

 protected:
  noncopyable() = default;
  ~noncopyable() = default;
};

} // namespace fm

#endif //LIBFM_BASE_NONCOAPYABLE_H_
