#pragma once

#include "shading.hpp"

#include "math/simd/float8.hpp"
#include "math/simd/vector4.hpp"

template<typename T>
struct traversal_ray_t {
  vector8_t  origin;
  vector8_t  direction;
  vector8_t  ood;
  float8_t   d;
  T*         segment;

  inline traversal_ray_t()
  {}

  inline traversal_ray_t(const traversal_ray_t& cpy)
    : origin(cpy.origin)
    , direction(cpy.direction)
    , ood(cpy.ood)
    , d(cpy.d)
  {}

  inline traversal_ray_t(const vector_t& p, const vector_t& dir, T* segment)
    : origin(p)
    , direction(dir)
    , ood(vector_t(
        1.0f/dir.x,
        1.0f/dir.y,
        1.0f/dir.z))
    , d(float8::load(segment->d))
    , segment(segment) 
  {}
};
