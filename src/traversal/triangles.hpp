#pragma once

#include "ray.hpp"
#include "math/simd/vector8.hpp"

/**
 * Moeller Trumbore triangle intersection tests. This serves as a baseline
 * for further experiments with other triangle intersection algorithms
 *
 */
struct moeller_trumbore_t {
  simd::max::vector8_t e0;
  simd::max::vector8_t e1;
  simd::max::vector8_t v0;

  triangle_t::p triangles[MAX_PRIMS_IN_NODE];

  inline moeller_trumbore_t(const triangle_t::p* tris, uint32_t num) {
    vector_t
      ve0[MAX_PRIMS_IN_NODE],
      ve1[MAX_PRIMS_IN_NODE],
      vv0[MAX_PRIMS_IN_NODE];

    for (int i=0; i<num; ++i) {
      triangles[i] = tris[i];

      ve0[i] = triangles[i]->v1() - triangles[i]->v0();
      ve1[i] = triangles[i]->v2() - triangles[i]->v0();
      vv0[i] = triangles[i]->v0();
    }

    e0 = simd::max::vector8_t(ve0);
    e1 = simd::max::vector8_t(ve1);
    v0 = simd::max::vector8_t(vv0);
  };

  inline bool intersect(traversal_ray_t& ray, shading_info_t& info) const {
    using namespace float8;
    using namespace vector8;

    const auto
      one  = load(1.0f),
      zero = load(0.0f),
      peps = load(0.00000001f),
      meps = load(-0.00000001f);

    const auto p   = cross(ray.direction, e1);
    const auto det = dot(e0, p);
    const auto ood = div(one, det);
    const auto t   = sub(ray.origin, v0);
    const auto q   = cross(t, e0);

    const auto u   = mul(dot(t, p), ood);
    const auto v   = mul(dot(ray.direction, q), ood);

    auto d = mul(dot(e1, q), ood);

    const auto xmask = mor(gt(det, peps), lt(det, meps));
    const auto umask = gte(u, zero);
    const auto vmask = mand(gte(v, zero), lte(add(u, v), one));
    const auto dmask = mand(gte(d, zero), lt(d, ray.d));

    auto mask = movemask(mand(mand(mand(vmask, umask), dmask), xmask));

    bool ret = false;
    
    if (mask != 0) {

      float dists[4];
      float closest = std::numeric_limits<float>::max();

      store(d, dists);

      int idx = -1;
      while(mask != 0) {
    	auto x = __bscf(mask);
    	if (dists[x] > 0.0f && dists[x] < closest && triangles[3-x]) {
    	  closest = dists[x];
    	  idx = x;
    	}
      }

      if (idx != -1) {
    	float us[4];
    	float vs[4];
    	store(u, us); store(v, vs);
    	ret = info.update(ray.ray, closest, triangles[3-idx].get(), us[idx], vs[idx]);
	
    	//d = _mm_min_ps(d, _mm_shuffle_ps(d, d, _MM_SHUFFLE(2, 1, 0, 3)));
        //d = _mm_min_ps(d, _mm_shuffle_ps(d, d, _MM_SHUFFLE(1, 0, 3, 2)));

    	ray.d = load(info.d);
      }
    }

    return ret;
  }
} __attribute__((aligned (16)));
