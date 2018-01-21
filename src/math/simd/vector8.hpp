#pragma once

#include "float8.hpp"
#include "../vector.hpp"

struct vector8_t {
  float8_t x, y, z;

  inline vector8_t()
  {}

  inline vector8_t(const vector8_t& cpy)
    : x(cpy.x), y(cpy.y), z(cpy.z)
  {}

  inline vector8_t(const vector_t& v) {
    x = float8::load(v.x);
    y = float8::load(v.y);
    z = float8::load(v.z);
  }

  inline vector8_t(const vector_t v[8]) {
    x = float8::load(v[0].x, v[1].x, v[2].x, v[3].x, v[4].x, v[5].x, v[6].x, v[7].x);
    y = float8::load(v[0].y, v[1].y, v[2].y, v[3].y, v[4].y, v[5].y, v[6].y, v[7].y);
    z = float8::load(v[0].z, v[1].z, v[2].z, v[3].z, v[4].z, v[5].z, v[6].z, v[7].z);
  }

  inline vector8_t(const float8_t& x, const float8_t& y, const float8_t& z)
    : x(x), y(y), z(z)
  {}
};

namespace vector8 {
#if defined(__SSE__)

  using namespace float8;

  inline vector8_t add(const vector8_t& l, const vector8_t& r) {
    vector8_t out = {
      _mm256_add_ps(l.x, r.x),
      _mm256_add_ps(l.y, r.y),
      _mm256_add_ps(l.z, r.z)
    };
    return out;
  }

  inline vector8_t sub(const vector8_t& l, const vector8_t& r) {
    vector8_t out = {
      _mm256_sub_ps(l.x, r.x),
      _mm256_sub_ps(l.y, r.y),
      _mm256_sub_ps(l.z, r.z)
    };
    return out;
  }

  inline vector8_t mul(const vector8_t& l, const vector8_t& r) {
    vector8_t out = {
      _mm256_mul_ps(l.x, r.x),
      _mm256_mul_ps(l.y, r.y),
      _mm256_mul_ps(l.z, r.z)
    };
    return out;
  }

  inline float8_t dot(const vector8_t& l, const vector8_t& r) {
    return madd(l.x, r.x, madd(l.y, r.y, float8::mul(l.z, r.z)));
  }

  inline vector8_t cross(const vector8_t& l, const vector8_t& r) {
    vector8_t out = {
      msub(l.y, r.z, float8::mul(l.z, r.y)),
      msub(l.z, r.x, float8::mul(l.x, r.z)),
      msub(l.x, r.y, float8::mul(l.y, r.x))
    };
    return out;
  }

#else
#error "No SIMD implementation for vector8_t available"
#endif
}
