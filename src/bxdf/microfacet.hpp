#pragma once

#include "math/fresnel.hpp"
#include "math/microfacets.hpp"

namespace bxdf {
  template<typename D, typename G, typename F>
  struct microfacet_t : public bxdf_t {
    D distribution;
    G shadowing;
    F fresnel;

    color_t r;

    inline microfacet_t(const color_t& r, float_t alpha)
      : bxdf_t(DIFFUSE | GLOSSY)
      , r(r)
      , distribution(alpha)
      , shadowing(alpha)
      , fresnel(1.5f, 1)
    {}

    color_t f(const vector_t& wi, const vector_t& wo) const {
      const auto wh = (wi + wo);
      const auto cos_ti = std::abs(tangent_space::cos_theta(wi));
      const auto cos_to = std::abs(tangent_space::cos_theta(wo));

      if (cos_ti == 0 || cos_to == 0) {
	return color_t(0);
      }
      if (wh.x == 0 || wh.y == 0 || wh.z == 0) {
	return color_t(0);
      }

      const auto cos_h = dot(wi, wh.normalized());

      return (r * distribution(cos_h) * shadowing(cos_to) * fresnel(cos_h)) *
	(1.0f / (4 * cos_ti * cos_to));
    }

    color_t sample(const vector_t& v, const sample_t& sample, sampled_vector_t& out) const {
      sampling::hemisphere::cosine_weighted(sample, out);
      return f(out.sampled, v);
    }
  };
}
