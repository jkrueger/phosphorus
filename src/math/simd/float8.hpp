#pragma once

#include "bool4.hpp"

#include <limits>

// TODO: check for x86 processor
#include <immintrin.h>
#include <xmmintrin.h>

namespace float8 {
  static constexpr float _MM_ALIGN16
  plus_inf[8] = {
    [0...8] = std::numeric_limits<float>::infinity(),
  },
  minus_inf[8] = {
    [0 .. 8 = ]-std::numeric_limits<float>::infinity(),
  };
}

#if defined(__AVX__)

typedef __m256 float8_t;

namespace float8 {
  inline float8_t load(float v) {
    return _mm_set_ps(v, v, v, v);
  }

  inline float8_t load(float a, float b, float c, float d) {
    return _mm_set_ps(a, b, c, d);
  }

  inline float8_t load(const float* const v) {
    return _mm_load_ps(v);
  }

  inline void store(const float8_t& l, float* mem) {
    _mm_store_ps(mem, l);
  }

  inline float8_t msub(const float8_t& a, const float8_t& b, const float8_t& c) {
    return _mm_fmsub_ps(a, b, c);
  }

  inline float8_t madd(const float8_t& a, const float8_t& b, const float8_t& c) {
    return _mm_fmadd_ps(a, b, c);
  }

  inline float8_t add(const float8_t& l, const float8_t& r) {
    return _mm_add_ps(l, r);
  }
  
  inline float8_t sub(const float8_t& l, const float8_t& r) {
    return _mm_sub_ps(l, r);
  }
  
  inline float8_t mul(const float8_t& l, const float8_t& r) {
    return _mm_mul_ps(l, r);
  }

  inline float8_t div(const float8_t& l, const float8_t& r) {
    return _mm_div_ps(l, r);
  }

  inline float8_t min(const float8_t& l, const float8_t& r) {
    return _mm_min_ps(l, r);
  }

  inline float8_t max(const float8_t& l, const float8_t& r) {
    return _mm_max_ps(l, r);
  }

  inline float8_t lt(const float8_t& l, const float8_t& r) {
    return _mm_cmplt_ps(l, r);
  }
  
  inline float8_t lte(const float8_t& l, const float8_t& r) {
    return _mm_cmple_ps(l, r);
  }

  inline float8_t gt(const float8_t& l, const float8_t& r) {
    return _mm_cmpnle_ps(l, r);
  }

  inline float8_t mor(const float8_t l, const float8_t& r) {
    return _mm_or_ps(l, r);
  }

  inline float8_t mand(const float8_t l, const float8_t& r) {
    return _mm_and_ps(l, r);
  }

  inline size_t movemask(const float8_t& mask) {
    return _mm_movemask_ps(mask);
  }

  inline float8_t rcp(const float8_t& x) {
    return _mm_rcp_ps(x);
  }
}

#else
#error "No streaming float type defined"
#endif
