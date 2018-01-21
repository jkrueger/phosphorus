#include "sphere.hpp"
#include "precision.hpp"

#include "math/orthogonal_base.hpp"

#include <math.h>

namespace parametric {

  sphere_t::sphere_t(float_t r)
    : radius(r), radius2(r*r)
  {}

  void sphere_t::sample(
    const vector_t& p,
    const sample_t* samples,
    sampled_vector_t* out,
    uint32_t num) const {

    for (auto i=0; i<num; ++i) {

      sampling::strategies::uniform_sample_hemisphere(samples[i], out[i]);

      out[i].pdf *= 1.0f / (radius2 * M_PI);
      out[i].sampled = out[i].sampled.scale(radius);
    }
  }
}
