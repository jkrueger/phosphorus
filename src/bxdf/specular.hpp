#pragma once

#include "shading.hpp"

#include <cmath>

namespace bxdf {
  struct simple_specular_t : public bxdf_t {
    double  s;
    color_t k;

    simple_specular_t()
      : bxdf_t(SPECULAR)
    {}

    inline void set(const color_t& _k, double _s) {
      k = _k; s = _s;
    }

    inline color_t f(const vector_t& in, const vector_t& out) const {
      return k * std::pow(normalize(out + in).y, s);
    }

    color_t sample(const vector_t&, const sample_t& sample, sampled_vector_t& out) const {
      sampling::strategies::cosine_sample_hemisphere(sample, out);
      return k * (1.0 / M_PI);
    }

    inline double pdf(const vector_t& in, const vector_t& out) const {
      return in.y * (1.0 / M_PI);
    }
  };
}
