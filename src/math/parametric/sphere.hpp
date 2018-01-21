#pragma once

#include "precision.hpp"

#include "math/sampling.hpp"
#include "math/vector.hpp"

namespace parametric {
  struct sphere_t {
    float_t radius;
    float_t radius2;

    sphere_t(float_t r);

    void sample(
      const vector_t& p,
      const sample_t* samples,
      sampled_vector_t* sampled,
      uint32_t num) const;
  };
}
