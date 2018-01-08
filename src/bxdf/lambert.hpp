#pragma once

#include "shading.hpp"

#include <algorithm>

namespace bxdf {
  struct lambert_t : public bxdf_t {

    color_t k;

    lambert_t()
      : bxdf_t(bxdf_t::DIFFUSE)
    {}

    color_t f(const vector_t& in, const vector_t& out) const {
      return k * (1.0 / M_PI);
    }

    color_t sample(const vector_t&, const sample_t& sample, sampled_vector_t& out) const {
      sampling::strategies::cosine_sample_hemisphere(sample, out);
      return k * (1.0 / M_PI);
    }

    double pdf(const vector_t& in, const vector_t& out) const {
      return in.y * (1.0 / M_PI);
    }
  };
}
