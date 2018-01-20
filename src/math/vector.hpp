#pragma once

#include "precision.hpp"

#include <iostream>

#include <math.h>

struct vector_t {
  union {
    struct {
      float_t x,y,z;
    };
    float_t v[3];
  };

  inline vector_t()
    : x(0), y(0), z(0)
  {}

  inline vector_t(const vector_t& cpy)
    : x(cpy.x), y(cpy.y), z(cpy.z)
  {}

  inline vector_t(float_t i)
    : x(i), y(i), z(i)
  {}

  inline vector_t(float_t _x, float_t _y, float_t _z)
    : x(_x), y(_y), z(_z)
  {}

  inline vector_t& scale(float_t s) {
    x *= s; y *= s; z *= s;
    return *this;
  }

  inline float_t length2() const {
    return dot(*this, *this);
  }

  inline float_t length() const {
    return sqrt(length2());
  }

  inline vector_t& normalize() {
    auto l2 = length2();
    if (l2 == 0.0) {
      return *this;
    }
    auto ool = 1.0 / sqrt(l2);
    return scale(ool);
  }

  inline vector_t normalized() const {
    vector_t out;
    out.normalize();
    return out;
  }

  inline friend float_t dot(const vector_t& l, const vector_t& r) {
    return l.x*r.x + l.y*r.y + l.z*r.z;
  }

  inline friend vector_t cross(const vector_t& l, const vector_t& r) {
    return
      vector_t(
        l.y*r.z - l.z*r.y,
	l.z*r.x - l.x*r.z,
	l.x*r.y - l.y*r.x);
  }
};

inline vector_t operator*(const vector_t& l, float_t r) {
  vector_t out(l);
  out.scale(r);
  return out;
}

inline vector_t operator*(float_t l, const vector_t& r) {
  vector_t out(r);
  out.scale(l);
  return out;
}

inline vector_t operator+(const vector_t& l, const vector_t& r) {
  return vector_t(l.x+r.x, l.y+r.y, l.z+r.z);
}

inline vector_t& operator+=(vector_t& l, const vector_t& r) {
  l.x+=r.x; l.y+=r.y; l.z+=r.z;
  return l;
}

inline vector_t operator+(const vector_t& l, float_t r) {
  return vector_t(l.x+r, l.y+r, l.z+r);
}

inline vector_t operator-(const vector_t& v) {
  return vector_t(v) * -1;
}

inline vector_t operator-(const vector_t& l, const vector_t& r) {
  return vector_t(l.x-r.x, l.y-r.y, l.z-r.z);
}

inline vector_t operator-(const vector_t& l, float_t r) {
  return vector_t(l.x-r, l.y-r, l.z-r);
}

inline float_t operator*(const vector_t& l, const vector_t& r) {
  return dot(l, r);
}

inline vector_t normalize(const vector_t& v) {
  return vector_t(v).normalize();
}

inline vector_t reflect(const vector_t& v, const vector_t& n) {
  return -v + 2 * dot(v, n) * n;
}

inline std::ostream& operator<<(std::ostream& o, const vector_t& v) {
  return o << "vector_t{" << v.x << ", " << v.y << ", " << v.z << "}";
}
