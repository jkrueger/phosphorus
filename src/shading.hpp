#pragma once

#include "material.hpp"
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
  float_t  d;
  uint32_t flags : 8, depth : 8, material: 16;
  float_t  u;
  float_t  v;
  uint32_t mesh;
  uint32_t face;

  inline segment_t()
    : beta(1.0f)
    , d(std::numeric_limits<float>::max())
    , flags(1)
    , depth(0)
  {}

  inline void kill() {
    flags = 0;
  }

  inline void revive() {
    flags = 1;
  }

  inline bool alive() const {
    return flags;
  }

  inline void follow() {
    p = p + d * wi;
    d = std::numeric_limits<float>::max();
  }

  inline void offset() {
    float_t offset = 0.0001f;
    if (!in_same_hemisphere(wi, n)) {
      offset = -offset;
    }
    p = p + n * offset;
  }
};

struct by_material_t {
  material_t* material;

  uint32_t    num;
  segment_t*  segments[4096];
  uint32_t    splats[4096];
};
