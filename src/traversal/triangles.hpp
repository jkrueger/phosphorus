#pragma once

#include "ray.hpp"
#include "math/simd/vector8.hpp"
#include "things/mesh.hpp"

/**
 * Moeller Trumbore triangle intersection tests. This serves as a baseline
 * for further experiments with other triangle intersection algorithms
 *
 */
template<int N>
struct moeller_trumbore_t {
  vector8_t e0;
  vector8_t e1;
  vector8_t v0;

  uint32_t num;

  uint32_t meshid[N];
  uint32_t faceid[N];

  inline moeller_trumbore_t(triangle_t::p* tris, uint32_t num)
    : num(num) {
    vector_t ve0[N], ve1[N], vv0[N];

    for (int i=0; i<num; ++i) {
      const auto& triangle = tris[i];

      faceid[i] = triangle->id;
      meshid[i] = triangle->mesh->id;

      ve0[i] = triangle->v1() - triangle->v0();
      ve1[i] = triangle->v2() - triangle->v0();
      vv0[i] = triangle->v0();
    }
    e0 = vector8_t(ve0);
    e1 = vector8_t(ve1);
    v0 = vector8_t(vv0);
  };

  template<typename T>
  inline bool intersect(traversal_ray_t<T>& ray) const {
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

    const auto us  = mul(dot(t, p), ood);
    const auto vs  = mul(dot(ray.direction, q), ood);

    auto ds = mul(dot(e1, q), ood);

    const auto xmask = mor(gt(det, peps), lt(det, meps));
    const auto umask = gte(us, zero);
    const auto vmask = mand(gte(vs, zero), lte(add(us, vs), one));
    const auto dmask = mand(gte(ds, zero), lt(ds, ray.d));

    auto mask = movemask(mand(mand(mand(vmask, umask), dmask), xmask));

    bool ret = false;

    if (mask != 0) {

      float dists[N];
      float closest = ray.segment->d;

      store(ds, dists);

      int idx = -1;
      while(mask != 0) {
 	auto x = __bscf(mask);
    	if (dists[x] > 0.0f && dists[x] < closest && ((7-x) < num)) {
    	  closest = dists[x];
    	  idx = x;
    	}
      }

      if (idx != -1) {
	if (T::shade) {
	  float u[N];
	  float v[N];
	  store(us, u);
	  store(vs, v);

	  ray.segment->shading(u[idx], v[idx], meshid[7-idx], faceid[7-idx]);
	}

    	ray.segment->d = closest;
    	ray.d          = load(closest);

    	ret = true;
      }
    }

    return ret;
  }
};
