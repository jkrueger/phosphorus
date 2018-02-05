#pragma once

#include "shading.hpp"

#include "math/simd/float8.hpp"
#include "math/simd/vector4.hpp"

struct traversal_ray_t {
  const vector8_t origin;
  const vector8_t direction;
  const vector8_t ood;
  float8_t        d;

  inline traversal_ray_t(const segment_t& segment, float d)
    : origin(segment.p)
    , direction(segment.wi)
    , ood(vector_t(
        1.0f/segment.wi.x,
        1.0f/segment.wi.y,
        1.0f/segment.wi.z))
    , d(float8::load(d))
  {}
} __attribute__((aligned (16)));
