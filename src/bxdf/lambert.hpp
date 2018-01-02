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
      auto a = std::max(0.0, in.y);
      return k * a;
    }

    color_t sample(const vector_t&, sample_t& sample) const {
      cosine_sample_hemisphere(sample);
      return k * sample.pdf;
    }

    double pdf(const vector_t& in, const vector_t& out) const {
      return in.y;
    }
  };
}
