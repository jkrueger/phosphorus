#pragma once

#include "precision.hpp"
#include "math/ray.hpp"
#include "math/vector.hpp"
#include "util/color.hpp"

#include <limits>

struct segment_t {
  vector_t p;
  vector_t wi;
  vector_t wo;
  vector_t n;
  color_t  beta;
  uint32_t material : 16, depth: 16; 

  inline segment_t()
    : beta(1.0f)
    , depth(0)
  {}

  inline ray_t ray(const vector_t& dir) const {
    float_t offset = 0.0001f;
    if (!in_same_hemisphere(dir, n)) {
      offset = -offset;
    }
    return ray_t(p + n * offset, dir);
  }
};
