#pragma once

#include "precision.hpp"
#include "ray.hpp"
#include "vector.hpp"

#include "simd/float4.hpp"
#include "simd/vector4.hpp"

#include <iostream>
#include <cmath>
#include <limits>

#include <stdint.h>

struct aabb_t {
  vector_t min, max;
  
  inline aabb_t()
    : min(std::numeric_limits<float_t>::max())
    , max(std::numeric_limits<float_t>::lowest())
  {}

  inline aabb_t(const aabb_t& cpy)
    : min(cpy.min), max(cpy.max)
  {}

  inline aabb_t(const vector_t& a, const vector_t& b)
    : min(a), max(b)
  {}

  inline vector_t centroid() const {
    return (min + max) * 0.5;
  }

  inline float_t area() const {
    auto d = max - min;
    return 2.0 * (d.x * d.y + d.x * d.z + d.y * d.z);
  }

  inline uint32_t dominant_axis() const {
    uint32_t out = 0;
    float_t   x   = 0.0;
    for (auto i=0; i<3; ++i) {
      float_t extent = max.v[i] - min.v[i];
      if (extent > x) {
	out = i;
	x = extent;
      }
    }
    return out;
  }

  inline bool empty_on(uint32_t axis) const {
    return min.v[axis] == max.v[axis];
  }

#ifdef DOUBLE_PRECISION
  inline bool intersect(const ray_t& ray, const float_t* const ood, float_t& t) const {
    static const float_t g = std::tgamma(3.0);
    
    float_t t0 = 0, t1 = std::numeric_limits<float_t>::max();
    for (auto i = 0; i < 3; ++i) {
      float_t near = (min.v[i] - ray.origin.v[i]) * ood[i];
      float_t far  = (max.v[i] - ray.origin.v[i]) * ood[i];

      if (near > far) std::swap(near, far);

      far *= 1 + 2 * g;
      t0 = near > t0 ? near : t0;
      t1 = far < t1 ? far : t1;
      if (t0 > t1) { return false; }
      t = std::min(t, t0);
    }
    return true;
  }
#else
  inline bool intersect(const ray_t& ray, const float_t* const oodp, float_t& t) const {
    static constexpr float //_MM_ALIGN16
      ps_cst_plus_inf[4] = {
        std::numeric_limits<float>::infinity(),
	std::numeric_limits<float>::infinity(),
	std::numeric_limits<float>::infinity(),
	std::numeric_limits<float>::infinity()
    };
    static constexpr float //_MM_ALIGN16
      ps_cst_minus_inf[4] = {
        -std::numeric_limits<float>::infinity(),
	-std::numeric_limits<float>::infinity(),
	-std::numeric_limits<float>::infinity(),
	-std::numeric_limits<float>::infinity()
      };
    const __m128
      plus_inf	= _mm_load_ps(ps_cst_plus_inf),
      minus_inf	= _mm_load_ps(ps_cst_minus_inf);
    const __m128
      box_min	= _mm_loadu_ps((const float * const) min.v),
      box_max	= _mm_loadu_ps((const float * const) max.v),
      pos	= _mm_loadu_ps((const float * const) ray.origin.v),
      ood	= _mm_loadu_ps((const float * const) oodp);

    // use a div if inverted directions aren't available
    const __m128 l1 = _mm_mul_ps(_mm_sub_ps(box_min, pos), ood);
    const __m128 l2 = _mm_mul_ps(_mm_sub_ps(box_max, pos), ood);

    // the order we use for those min/max is vital to filter out
    // NaNs that happens when an inv_dir is +/- inf and
    // (box_min - pos) is 0. inf * 0 = NaN
    const __m128 filtered_l1a = _mm_min_ps(l1, plus_inf);
    const __m128 filtered_l2a = _mm_min_ps(l2, plus_inf);
    
    const __m128 filtered_l1b = _mm_max_ps(l1, minus_inf);
    const __m128 filtered_l2b = _mm_max_ps(l2, minus_inf);

    // now that we're back on our feet, test those slabs.
    __m128 lmax = _mm_max_ps(filtered_l1a, filtered_l2a);
    __m128 lmin = _mm_min_ps(filtered_l1b, filtered_l2b);

    // unfold back. try to hide the latency of the shufps & co.
    const __m128 lmax0 = _mm_shuffle_ps(lmax, lmax, 0x39);
    const __m128 lmin0 = _mm_shuffle_ps(lmin, lmin, 0x39);
    lmax = _mm_min_ss(lmax, lmax0);
    lmin = _mm_max_ss(lmin, lmin0);

    const __m128 lmax1 = _mm_movehl_ps(lmax,lmax);
    const __m128 lmin1 = _mm_movehl_ps(lmin,lmin);
    lmax = _mm_min_ss(lmax, lmax1);
    lmin = _mm_max_ss(lmin, lmin1);

    const bool ret = _mm_comige_ss(lmax, _mm_setzero_ps()) & _mm_comige_ss(lmax,lmin);

    _mm_store_ss((float*)&t, lmin);
    //_mm_store_ss(lmax, &rs.t_far);

    return  ret;
  }
#endif
};

namespace bounds {

  inline aabb_t& merge(aabb_t& l, const vector_t& r) {
    for (auto i=0; i<3; ++i) {
      l.min.v[i] = std::min(l.min.v[i], r.v[i]);
      l.max.v[i] = std::max(l.max.v[i], r.v[i]);
    }
    return l;
  }

  inline aabb_t merge(const aabb_t& l, const aabb_t& r) {
    aabb_t out(l);
    return merge(merge(out, r.min), r.max);
  }

  inline vector_t offset(const aabb_t& l, const vector_t& r) {
    auto o = r - l.min;
    if (l.max.x > l.min.x) { o.x /= (l.max.x - l.min.x); } 
    if (l.max.y > l.min.y) { o.y /= (l.max.y - l.min.y); } 
    if (l.max.z > l.min.z) { o.z /= (l.max.z - l.min.z); }
    return o;
  }

  template<int N>
  inline size_t intersect_all(
    const vector4_t& o, const vector4_t& ood, const float4_t& d,
    const float* const bounds, const uint32_t* indices,
    float4_t& dist);

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
}

/*
#if defined(__AVX2__)
  tmeplate<>
  inline void intersect_all<8>(const ray_t& ray, const float* const boxes, const float* const ood) {
  }
#else
  #error "No streamed implementation for AABB check available"
#endif
}
*/

inline std::ostream& operator<<(std::ostream& o, const aabb_t& a) {
  return o << "aabb_t{" << "min=" << a.min << ", max=" << a.max << "}";
}
