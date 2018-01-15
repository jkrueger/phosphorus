#pragma once

#include "precision.hpp"
#include "math/vector.hpp"

struct ray_t {
  vector_t origin;
  vector_t direction;
  uint64_t padding;

  inline ray_t()
  {}

  inline ray_t(const ray_t& cpy)
    : origin(cpy.origin), direction(cpy.direction)
  {}
  
  inline ray_t(const vector_t& o, const vector_t& d)
    : origin(o), direction(d)
  {}

  inline vector_t at(float_t d) const {
    return origin + direction * d;
  }
};
