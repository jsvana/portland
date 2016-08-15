#pragma once

#include <memory>
#include <ostream>

namespace util {

  template <typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

  /**
   * Generic function to clamp a number inside the given min and max
   *
   * @param num Number to clamp
   * @param min Minimum value for number
   * @param max Maximum value for numver
   * @return Whether number was clamped
   */
  template <typename T>
  bool clamp(T& num, T min, T max) {
    T clamped = std::max(min, std::min(num, max));
    if (clamped != num) {
      num = clamped;
      return true;
    }
    return false;
  }

}  // namespace util
