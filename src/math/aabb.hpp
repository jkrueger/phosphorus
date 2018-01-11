#pragma once

#include "ray.hpp"
#include "vector.hpp"

#include <iostream>
#include <cmath>
#include <limits>

#include <stdint.h>

struct aabb_t {
  vector_t min, max;
  
  inline aabb_t()
    : min(std::numeric_limits<double>::max())
    , max(std::numeric_limits<double>::lowest())
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

  inline double area() const {
    auto d = max - min;
    return 2.0 * (d.x * d.y + d.x * d.z + d.y * d.z);
  }

  inline uint32_t dominant_axis() const {
    uint32_t out = 0;
    double   x   = 0.0;
    for (auto i=0; i<3; ++i) {
      double extent = max.v[i] - min.v[i];
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

  inline bool intersect(const ray_t& ray, double ood[3], double& t) const {
    static const double g = std::tgamma(3.0);
    
    double t0 = 0, t1 = std::numeric_limits<double>::max();
    for (auto i = 0; i < 3; ++i) {
      double near = (min.v[i] - ray.origin.v[i]) * ood[i];
      double far  = (max.v[i] - ray.origin.v[i]) * ood[i];

      if (near > far) std::swap(near, far);

      far *= 1 + 2 * g;
      t0 = near > t0 ? near : t0;
      t1 = far < t1 ? far : t1;
      if (t0 > t1) { return false; }
      t = std::min(t, t0);
    }
    return true;
  }
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
}

inline std::ostream& operator<<(std::ostream& o, const aabb_t& a) {
  return o << "aabb_t{" << "min=" << a.min << ", max=" << a.max << "}";
}
