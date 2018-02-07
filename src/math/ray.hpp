#pragma once

#include "precision.hpp"
#include "math/vector.hpp"

struct ray_t {
  vector_t origin;
  vector_t direction;
  float_t  d;
  uint32_t material;

  inline ray_t()
    : d(std::numeric_limits<float>::max())
  {}

  inline ray_t(const ray_t& cpy)
    : origin(cpy.origin), direction(cpy.direction)
  {}
  
  inline ray_t(const vector_t& o, const vector_t& d)
    : origin(o), direction(d)
  {}

  inline vector_t p() const {
    return origin + direction * d;
  }
};
