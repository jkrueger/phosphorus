#pragma once

#include "simd/float4.hpp"
#include "simd/float8.hpp"
#include "simd/vector4.hpp"
#include "simd/vector8.hpp"

template<int Width>
struct simd_t {
};

template<>
struct simd_t<4>{
  typedef float4_t  float_t;
  typedef vector4_t vector_t;
};

template<>
struct simd_t<8>{
  typedef float8_t  float_t;
  typedef vector8_t vector_t;
};
