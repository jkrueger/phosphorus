#pragma once

#include "precision.hpp"
#include "bxdf.hpp"

#include <algorithm>

namespace bxdf {
  struct lambert_t : public bxdf_t {

    color_t k;

    lambert_t(const color_t& k)
      : bxdf_t(bxdf_t::DIFFUSE | bxdf_t::REFLECTIVE), k(k)
    {}

    color_t f(const vector_t& in, const vector_t& out) const {
      return k * (1.0 / M_PI);
    }

    color_t sample(const vector_t&, const sample_t& sample, sampled_vector_t& out) const {
      sampling::hemisphere::cosine_weighted(sample, out);
      return k * (1.0 / M_PI);
    }

    float_t pdf(const vector_t& in, const vector_t& out) const {
      return in.y * (1.0 / M_PI);
    }
  };
}
