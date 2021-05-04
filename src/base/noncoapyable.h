//
// Created by Ye-zixiao on 2021/4/7.
//

#ifndef FAKEMUDUO_BASE_NONCOAPYABLE_H_
#define FAKEMUDUO_BASE_NONCOAPYABLE_H_

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

#endif //FAKEMUDUO_BASE_NONCOAPYABLE_H_
