//
// Created by Ye-zixiao on 2021/7/25.
//

#ifndef LIBFM_LIBFM_BASE_MOVABLE_H_
#define LIBFM_LIBFM_BASE_MOVABLE_H_

namespace fm {

class Movable {
 public:
  Movable() = default;
  ~Movable() = default;

  Movable(Movable &&) noexcept = default;
  Movable &operator=(Movable &&) noexcept = default;

  Movable(const Movable &) = delete;
  Movable &operator=(const Movable &) = delete;
};

} // namespace fm

#endif //LIBFM_LIBFM_BASE_MOVABLE_H_
