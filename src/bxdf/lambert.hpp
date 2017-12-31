#pragma once

#include "shading.hpp"

#include <algorithm>

namespace bxdf {
  struct lambert_t : public bxdf_t {

    color_t k;

    inline color_t f(const vector_t& in, const vector_t& out, const vector_t& n) const {

      auto a = std::max(0.0, dot(n, in));
      return k * a;
    }
  };
}
