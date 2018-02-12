#pragma once

#include "shading.hpp"

#include "math/simd/float8.hpp"
#include "math/simd/vector4.hpp"

struct traversal_ray_t {
  const vector8_t origin;
  const vector8_t direction;
  const vector8_t ood;
  float8_t        d;

  inline traversal_ray_t(const vector_t& p, const vector_t& dir, float d)
    : origin(p)
    , direction(dir)
    , ood(vector_t(
        1.0f/dir.x,
        1.0f/dir.y,
        1.0f/dir.z))
    , d(float8::load(d))
  {}
};
