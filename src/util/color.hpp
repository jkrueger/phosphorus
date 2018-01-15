#pragma once

#include "precision.hpp"

struct color_t {
  float_t r, g, b;

  inline color_t()
    : r(0), g(0), b(0)
  {}

  inline color_t(float_t x)
    : r(x), g(x), b(x)
  {}

  inline color_t(const color_t& cpy)
    : r(cpy.r), g(cpy.g), b(cpy.b)
  {}

  inline color_t(float_t _r, float_t _g, float_t _b)
    : r(_r), g(_g), b(_b)
  {}

  inline color_t& scale(float_t s) {
    r *= s; g *= s; b *= s;
    return *this;
  }

  inline float_t y() const {
    static const float_t weight[3] = {0.212671, 0.715160, 0.072169};
    return weight[0] * r + weight[1] * g + weight[2] * b;
  }
};

inline color_t operator+(const color_t& l, const color_t& r) {
  color_t out(l);
  out.r += r.r; out.g += r.g; out.b += r.b;
  return out;
}

inline color_t& operator+=(color_t& l, const color_t& r) {
  l.r += r.r; l.g += r.g; l.b += r.b;
  return l;
}

inline color_t operator-(const color_t& l, const color_t& r) {
  color_t out(l);
  out.r -= r.r; out.g -= r.g; out.b -= r.b;
  return out;
}

inline color_t operator*(const color_t& l, float_t r) {
  color_t out(l);
  out.scale(r);
  return out;
}

inline color_t& operator*=(color_t& l, const color_t& r) {
  l.r *= r.r; l.g *= r.g; l.b *= r.b;
  return l;
}

inline color_t operator*(const color_t& l, const color_t& r) {
  return color_t(l.r*r.r, l.g*r.g, l.b*r.b);
}
