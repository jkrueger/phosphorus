#pragma once

#include "math/simd/float8.hpp"
#include "math/simd/vector4.hpp"

struct traversal_ray_t {
  const vector8_t origin;
  const vector8_t direction;
  const vector8_t ood;
  float8_t        d;
  const ray_t&    ray;

  inline traversal_ray_t(const ray_t& ray, float d)
    : ray(ray)
    , origin(ray.origin)
    , direction(ray.direction)
    , ood(vector_t(
        1.0f/ray.direction.x,
        1.0f/ray.direction.y,
        1.0f/ray.direction.z))
    , d(float8::load(d))
  {}
} __attribute__((aligned (16)));
