#pragma once

#include "bool4.hpp"

#include <limits>

// TODO: check for x86 processor
#include <immintrin.h>
#include <xmmintrin.h>

namespace float4 {
  static constexpr float //_MM_ALIGN16
  plus_inf[] = {
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity(),
    std::numeric_limits<float>::infinity()
  };
  static constexpr float //_MM_ALIGN16
  minus_inf[] = {
    -std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity(),
    -std::numeric_limits<float>::infinity()
  };
}

#if defined(__SSE__)

typedef __m128 float4_t;

namespace float4 {
  inline __m128 load(float v) {
    return _mm_set_ps(v, v, v, v);
  }

  inline float4_t load(float a, float b, float c, float d) {
    return _mm_set_ps(a, b, c, d);
  }

  inline float4_t load(const float* const v) {
    return _mm_load_ps(v);
  }

  inline void store(const float4_t& l, float* mem) {
    _mm_store_ps(mem, l);
  }

  inline float4_t msub(const float4_t& a, const float4_t& b, const float4_t& c) {
#if defined(__SSE_4_2__)
    return _mm_fmsub_ps(a, b, c);
#else
    return _mm_sub_ps(_mm_mul_ps(a, b), c);
#endif
  }

  inline float4_t madd(const float4_t& a, const float4_t& b, const float4_t& c) {
#if defined(__SSE_4_2__)
    return _mm_fmadd_ps(a, b, c);
#else
    return _mm_add_ps(_mm_mul_ps(a, b), c);
#endif
  }

  inline float4_t add(const float4_t& l, const float4_t& r) {
    return _mm_add_ps(l, r);
  }
  
  inline float4_t sub(const float4_t& l, const float4_t& r) {
    return _mm_sub_ps(l, r);
  }
  
  inline float4_t mul(const float4_t& l, const float4_t& r) {
    return _mm_mul_ps(l, r);
  }

  inline float4_t div(const float4_t& l, const float4_t& r) {
    return _mm_div_ps(l, r);
  }

  inline float4_t min(const float4_t& l, const float4_t& r) {
    return _mm_min_ps(l, r);
  }

  inline float4_t max(const float4_t& l, const float4_t& r) {
    return _mm_max_ps(l, r);
  }

  inline float4_t lt(const float4_t& l, const float4_t& r) {
    return _mm_cmplt_ps(l, r);
  }
  
  inline float4_t lte(const float4_t& l, const float4_t& r) {
    return _mm_cmple_ps(l, r);
  }

  inline float4_t gt(const float4_t& l, const float4_t& r) {
    return _mm_cmpnle_ps(l, r);
  }

  inline float4_t gte(const float4_t& l, const float4_t& r) {
    return _mm_cmpnlt_ps(l, r);
  }

  inline float4_t mor(const float4_t l, const float4_t& r) {
    return _mm_or_ps(l, r);
  }

  inline float4_t mand(const float4_t l, const float4_t& r) {
    return _mm_and_ps(l, r);
  }

  inline size_t movemask(const float4_t& mask) {
    return _mm_movemask_ps(mask);
  }

  inline float4_t rcp(const float4_t& x) {
    return _mm_rcp_ps(x);
  }
}

#else
#error "No streaming float type defined"
#endif
