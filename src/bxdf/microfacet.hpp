#pragma once

#include "math/microfacet.hpp"

namespace bxdf {
  template<typename D, typename G, typename F>
  struct microfacet_t {
    D distribution;
    G shadowing;
    F fresnel;

    color_t r;

    inline microfacet_t(const color_t& r)
      : r(r)
    {}

    color_t f(const vector_t& wi, const vector_t& wo) const {
      const auto wh = wi + wo;
      const auto cos_ti = std::abs(tagent_space::cos_theta(wi));
      const auto cos_to = std::abs(tagent_space::cos_theta(wo));
      const auto cos_h  = dot(wi, wh);
      wh.normalize();

      return (r * distribution(wh) * shadowing(wo) * fresnel(cos_h)) /
	(4 * cos_ti * cos_to);
    }

    color_t sample(const vector_t& v, const sample_t& sample, sampled_vector_t& out) const {
      sampling::hemisphere::cosine_weighted(sample, out);
      return f(out.sampled, v);
    }
  };
}
