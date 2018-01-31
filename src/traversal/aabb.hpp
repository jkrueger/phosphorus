#pragma once

#include "math/simd.hpp"
#include "math/simd/float4.hpp"
#include "math/simd/vector4.hpp"
#include "math/simd/float8.hpp"
#include "math/simd/vector8.hpp"

namespace bounds {
  template<int N>
  inline size_t intersect_all(
    const typename simd_t<N>::vector_t& o,
    const typename simd_t<N>::vector_t& ood,
    const typename simd_t<N>::float_t& d,
    const float* const bounds, const uint32_t* indices,
    typename simd_t<N>::float_t& dist);

  template<>
  inline size_t intersect_all<4>(
    const vector4_t& o, const vector4_t& ood, const float4_t& d,
    const float* const bounds, const uint32_t* indices,
    float4_t& dist) {

    using namespace float4;

    // eventually tnear/tfar values
    const float4_t zero = load(0.0f);

    const float4_t min_x = mul(sub(load(&bounds[indices[0]]), o.x), ood.x);
    const float4_t min_y = mul(sub(load(&bounds[indices[1]]), o.y), ood.y);
    const float4_t min_z = mul(sub(load(&bounds[indices[2]]), o.z), ood.z);

    const float4_t max_x = mul(sub(load(&bounds[indices[3]]), o.x), ood.x);
    const float4_t max_y = mul(sub(load(&bounds[indices[4]]), o.y), ood.y);
    const float4_t max_z = mul(sub(load(&bounds[indices[5]]), o.z), ood.z);

    const float4_t n = max(max(min_x, min_y), max(min_z, zero));
    const float4_t f = min(min(max_x, max_y), min(max_z, d));

    const auto mask = lte(n, f);

    dist = n;

    return movemask(mask);
  }

  template<>
  inline size_t intersect_all<8>(
    const vector8_t& o, const vector8_t& ood, const float8_t& d,
    const float* const bounds, const uint32_t* indices,
    float8_t& dist) {

    using namespace float8;

    // eventually tnear/tfar values
    const float8_t zero = load(0.0f);

    const float8_t min_x = mul(sub(load(&bounds[indices[0]]), o.x), ood.x);
    const float8_t min_y = mul(sub(load(&bounds[indices[1]]), o.y), ood.y);
    const float8_t min_z = mul(sub(load(&bounds[indices[2]]), o.z), ood.z);

    const float8_t max_x = mul(sub(load(&bounds[indices[3]]), o.x), ood.x);
    const float8_t max_y = mul(sub(load(&bounds[indices[4]]), o.y), ood.y);
    const float8_t max_z = mul(sub(load(&bounds[indices[5]]), o.z), ood.z);

    const float8_t n = max(max(min_x, min_y), max(min_z, zero));
    const float8_t f = min(min(max_x, max_y), min(max_z, d));

    const auto mask = lte(n, f);

    dist = n;

    return movemask(mask);
  }
}
