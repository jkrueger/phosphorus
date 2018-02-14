#pragma once

#include "math/simd.hpp"
#include "math/simd/float4.hpp"
#include "math/simd/vector4.hpp"
#include "math/simd/float8.hpp"
#include "math/simd/vector8.hpp"

namespace bounds {
  template<int N>
  struct bounds_t {
  };

  template<>
  struct bounds_t<4> {
    float4_t min[3];
    float4_t max[3];
  };

  template<>
  struct __attribute__((aligned (64))) bounds_t<8> {
    float8_t min[3];
    float8_t max[3];
  };

  template<int N>
  inline bounds_t<N> load(
    const float_t* const bounds
  , const uint32_t* const indices)
  {
    bounds_t<N> out;
    out.min[0] = float8::load(bounds[indices[0]]);
    out.min[1] = float8::load(bounds[indices[1]]);
    out.min[2] = float8::load(bounds[indices[2]]);
    out.max[0] = float8::load(bounds[indices[3]]);
    out.max[1] = float8::load(bounds[indices[4]]);
    out.max[2] = float8::load(bounds[indices[5]]);

    return out;
  }

  template<int N>
  inline float8_t intersect_all(
    const typename simd_t<N>::vector_t& o,
    const typename simd_t<N>::vector_t& ood,
    const typename simd_t<N>::float_t& d,
    const bounds_t<N>& bounds,
    typename simd_t<N>::float_t& dist);
  /*
  template<>
  inline float4_t intersect_all<4>(
    const vector4_t& o, const vector4_t& ood, const float4_t& d,
    const bounds_t<4>& bounds,
    float4_t& dist) {

    using namespace float4;

    const float4_t zero = float4::load(0.0f);

    const float4_t min_x = mul(sub(bounds.min[0], o.x), ood.x);
    const float4_t min_y = mul(sub(bounds.min[1], o.y), ood.y);
    const float4_t min_z = mul(sub(bounds.min[2], o.z), ood.z);

    const float4_t max_x = mul(sub(bounds.max[0], o.x), ood.x);
    const float4_t max_y = mul(sub(bounds.max[1], o.y), ood.y);
    const float4_t max_z = mul(sub(bounds.max[2], o.z), ood.z);

    const float4_t n = max(max(min_x, min_y), max(min_z, zero));
    const float4_t f = min(min(max_x, max_y), min(max_z, d));

    const auto mask = lte(n, f);

    dist = n;

    return mask;
  }
  */
  template<>
  inline float8_t intersect_all<8>(
    const vector8_t& o, const vector8_t& ood, const float8_t& d,
    const bounds_t<8>& bounds,
    float8_t& dist) {

    using namespace float8;

    const float8_t zero = float8::load(0.0f);

    const auto ltz_x = lt(ood.x, zero);
    const auto ltz_y = lt(ood.y, zero);
    const auto ltz_z = lt(ood.z, zero);

    auto min_x = mor(mand(ltz_x, bounds.max[0]), andnot(ltz_x, bounds.min[0]));
    auto min_y = mor(mand(ltz_y, bounds.max[1]), andnot(ltz_y, bounds.min[1]));
    auto min_z = mor(mand(ltz_z, bounds.max[2]), andnot(ltz_z, bounds.min[2]));
    auto max_x = mor(mand(ltz_x, bounds.min[0]), andnot(ltz_x, bounds.max[0]));
    auto max_y = mor(mand(ltz_y, bounds.min[1]), andnot(ltz_y, bounds.max[1]));
    auto max_z = mor(mand(ltz_z, bounds.min[2]), andnot(ltz_z, bounds.max[2]));

    min_x = mul(sub(min_x, o.x), ood.x);
    min_y = mul(sub(min_y, o.y), ood.y);
    min_z = mul(sub(min_z, o.z), ood.z);

    max_x = mul(sub(max_x, o.x), ood.x);
    max_y = mul(sub(max_y, o.y), ood.y);
    max_z = mul(sub(max_z, o.z), ood.z);

    const float8_t n = max(max(min_x, min_y), max(min_z, zero));
    const float8_t f = min(min(max_x, max_y), min(max_z, d));

    const auto mask = lte(n, f);

    dist = n;

    return mask;
  }
}
