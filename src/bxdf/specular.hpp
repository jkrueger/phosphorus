#pragma once

#include "precision.hpp"
#include "bxdf.hpp"

#include <cmath>

namespace bxdf {
  struct simple_specular_t : public bxdf_t {
    float_t s;
    color_t k;

    simple_specular_t(const color_t& k, float_t s)
      : bxdf_t(SPECULAR), k(k), s(s)
    {}

    inline void set(const color_t& _k, float_t _s) {
      k = _k; s = _s;
    }

    inline color_t f(const vector_t& in, const vector_t& out) const {
      return k * std::pow(normalize(out + in).y, s);
    }

    color_t sample(const vector_t&, const sample_t& sample, sampled_vector_t& out) const {
      sampling::hemisphere::cosine_weighted(sample, out);
      return k * (1.0 / M_PI);
    }

    inline float_t pdf(const vector_t& in, const vector_t& out) const {
      return in.y * (1.0 / M_PI);
    }
  };
}
