#pragma once

#include "math/simd/float4.hpp"
#include "math/simd/float8.hpp"
#include "math/simd/vector4.hpp"

struct traversal_ray_t {
  const vector4_t origin;
  const vector4_t direction;
  const vector4_t ood;
  float4_t d;
  const ray_t&    ray;

  inline traversal_ray_t(const ray_t& ray, float d)
    : ray(ray)
    , origin(ray.origin)
    , direction(ray.direction)
    , ood(vector_t(
        1.0f/ray.direction.x,
        1.0f/ray.direction.y,
        1.0f/ray.direction.z))
    , d(float4::load(d))
  {}
} __attribute__((aligned (16)));
