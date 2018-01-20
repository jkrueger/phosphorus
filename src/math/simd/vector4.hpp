#pragma once

#include "float4.hpp"
#include "../vector.hpp"

struct vector4_t {
  float4_t x, y, z;

  inline vector4_t()
  {}

  inline vector4_t(const vector4_t& cpy)
    : x(cpy.x), y(cpy.y), z(cpy.z)
  {}

  inline vector4_t(const vector_t& v) {
    x = float4::load(v.x);
    y = float4::load(v.y);
    z = float4::load(v.z);
  }

  inline vector4_t(const vector_t v[4]) {
    x = float4::load(v[0].x, v[1].x, v[2].x, v[3].x);
    y = float4::load(v[0].y, v[1].y, v[2].y, v[3].y);
    z = float4::load(v[0].z, v[1].z, v[2].z, v[3].z);
  }

  inline vector4_t(const float4_t& x, const float4_t& y, const float4_t& z)
    : x(x), y(y), z(z)
  {}
};

namespace vector4 {
#if defined(__SSE__)

  using namespace float4;

  inline vector4_t add(const vector4_t& l, const vector4_t& r) {
    vector4_t out = {
      _mm_add_ps(l.x, r.x),
      _mm_add_ps(l.y, r.y),
      _mm_add_ps(l.z, r.z)
    };
    return out;
  }

  inline vector4_t sub(const vector4_t& l, const vector4_t& r) {
    vector4_t out = {
      _mm_sub_ps(l.x, r.x),
      _mm_sub_ps(l.y, r.y),
      _mm_sub_ps(l.z, r.z)
    };
    return out;
  }

  inline vector4_t mul(const vector4_t& l, const vector4_t& r) {
    vector4_t out = {
      _mm_mul_ps(l.x, r.x),
      _mm_mul_ps(l.y, r.y),
      _mm_mul_ps(l.z, r.z)
    };
    return out;
  }

  inline float4_t dot(const vector4_t& l, const vector4_t& r) {
    return madd(l.x, r.x, madd(l.y, r.y, float4::mul(l.z, r.z)));
  }

  inline vector4_t cross(const vector4_t& l, const vector4_t& r) {
    vector4_t out = {
      msub(l.y, r.z, float4::mul(l.z, r.y)),
      msub(l.z, r.x, float4::mul(l.x, r.z)),
      msub(l.x, r.y, float4::mul(l.y, r.x))
    };
    return out;
  }

#else
#error "No SIMD implementation for vector4_t available"
#endif
}
