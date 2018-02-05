#pragma once

#include <algorithm>

template<typename T>
inline constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
  return std::max(lo, std::min(v, hi));
}

template<typename T>
inline constexpr T square(const T& x) {
  return x*x;
}
