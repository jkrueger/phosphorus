#pragma once

#include "bool4.hpp"

#include <limits>

// TODO: check for x86 processor
#include <immintrin.h>
#include <xmmintrin.h>

//#define ATTRS __attribute__((__target__("xsave"), __target__("avx"), __target__("fma")))
#define ATTRS

namespace float8 {
  static constexpr float _MM_ALIGN16
  plus_inf[8] = {
    [0 ... 7] = std::numeric_limits<float>::infinity(),
  },
  minus_inf[8] = {
    [0 ... 7] = -std::numeric_limits<float>::infinity(),
  };
}

//#if defined(__AVX__)

typedef __m256 float8_t;

namespace float8 {
  inline float8_t ATTRS load(float v) {
    return _mm256_set_ps(v, v, v, v, v, v, v, v);
  }

  inline float8_t ATTRS load(float a, float b, float c, float d, float e, float f, float g, float h) {
    return _mm256_set_ps(a, b, c, d, e, f, g, h);
  }

  inline float8_t ATTRS load(const float* const v) {
    return _mm256_load_ps(v);
  }

  inline void ATTRS store(const float8_t& l, float* mem) {
    _mm256_store_ps(mem, l);
  }

  inline float8_t ATTRS msub(const float8_t& a, const float8_t& b, const float8_t& c) {
    return _mm256_fmsub_ps(a, b, c);
  }

  inline float8_t ATTRS madd(const float8_t& a, const float8_t& b, const float8_t& c) {
    return _mm256_fmadd_ps(a, b, c);
  }

  inline float8_t ATTRS add(const float8_t& l, const float8_t& r) {
    return _mm256_add_ps(l, r);
  }

  inline float8_t ATTRS sub(const float8_t& l, const float8_t& r) {
    return _mm256_sub_ps(l, r);
  }
  
  inline float8_t ATTRS mul(const float8_t& l, const float8_t& r) {
    return _mm256_mul_ps(l, r);
  }

  inline float8_t ATTRS div(const float8_t& l, const float8_t& r) {
    return _mm256_div_ps(l, r);
  }

  inline float8_t ATTRS min(const float8_t& l, const float8_t& r) {
    return _mm256_min_ps(l, r);
  }

  inline float8_t ATTRS max(const float8_t& l, const float8_t& r) {
    return _mm256_max_ps(l, r);
  }

  inline float8_t ATTRS lt(const float8_t& l, const float8_t& r) {
    return _mm256_cmp_ps(l, r, _CMP_LT_OS);
  }

  inline float8_t ATTRS lte(const float8_t& l, const float8_t& r) {
    return _mm256_cmp_ps(l, r, _CMP_LE_OS);
  }

  inline float8_t ATTRS gt(const float8_t& l, const float8_t& r) {
    return _mm256_cmp_ps(l, r, _CMP_GT_OS);
  }

  inline float8_t ATTRS gte(const float8_t& l, const float8_t& r) {
    return _mm256_cmp_ps(l, r, _CMP_GE_OS);
  }

  inline float8_t ATTRS mor(const float8_t l, const float8_t& r) {
    return _mm256_or_ps(l, r);
  }

  inline float8_t ATTRS mand(const float8_t l, const float8_t& r) {
    return _mm256_and_ps(l, r);
  }

  inline size_t ATTRS movemask(const float8_t& mask) {
    return _mm256_movemask_ps(mask);
  }

  inline float8_t rcp(const float8_t& x) {
    return _mm256_rcp_ps(x);
  }
}

//#else
//#error "No streaming float type defined"
//#endif
