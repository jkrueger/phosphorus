#pragma once

struct color_t {
  double r, g, b;

  inline color_t()
    : r(0), g(0), b(0)
  {}

  inline color_t(const color_t& cpy)
    : r(cpy.r), g(cpy.g), b(cpy.b)
  {}

  inline color_t(double _r, double _g, double _b)
    : r(_r), g(_g), b(_b)
  {}

  inline color_t& scale(double s) {
    r *= s; g *= s; b *= s;
    return *this;
  }
};

inline color_t& operator+=(color_t& l, const color_t& r) {
  l.r += r.r; l.g += r.g; l.b += r.b;
  return l;
}

inline color_t operator*(const color_t& l, double r) {
  color_t out(l);
  out.scale(r);
  return out;
}

inline color_t operator*(const color_t& l, const color_t& r) {
  return color_t(l.r*r.r, l.g*r.g, l.b*r.b);
}
