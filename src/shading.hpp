#pragma once

#include "material.hpp"
#include "precision.hpp"
#include "math/ray.hpp"
#include "math/vector.hpp"
#include "util/color.hpp"

#include <limits>

struct segment_t {
  vector_t p; // 12
  vector_t wi; // 24
  vector_t n;  // 32
  color_t  beta; // 44
  float_t  d; // 48
  float_t  u; // 52
  float_t  v; // 56
  uint32_t mesh; // 60
  uint32_t face; // 64
  uint32_t flags : 8, depth : 8, material: 16; // 68
  vector_t wo; // 80
  char     padding[48];

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
