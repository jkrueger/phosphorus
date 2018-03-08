#pragma once

#include "precision.hpp"
#include "trig.hpp"
#include "vector.hpp"
#include "util/algo.hpp"

namespace microfacet {
  namespace distribution {
    /**
     * Trowbridge-Reitz microfacet distribution
     *
     */
    struct ggx_t {
      float_t alpha;

      inline ggx_t(float_t alpha)
	: alpha(alpha)
      {}

      inline float_t operator()(const float_t cos_h) const {
	const auto a2  = square(alpha);
	const auto nm2 = square(cos_h);

	return a2/(M_PI*square(nm2*(a2-1)+1));
      }

      // inline vector_t sample() const {}

      static inline float_t roughness_to_alpha(float_t roughness) {
	//return square(roughness);
	roughness = std::max(roughness, (float_t)1e-3);
	float_t x = std::log(roughness);
	return 1.62142f
	  + 0.819955f * x
	  + 0.1734f * x * x
	  + 0.0171201f * x * x * x
	  + 0.000640711f * x * x * x * x;
      }
    };

    struct ggx_pbrt_t {
      float_t alpha;

      inline ggx_pbrt_t(float_t alpha)
	: alpha(alpha)
      {}

      inline float_t operator()(const vector_t& wh) const {
	const auto tan2_t = tangent_space::tan2_theta(wh);
	if (std::isinf(tan2_t)) return 0.;
	const auto cos4_t = tangent_space::cos2_theta(wh) * tangent_space::cos2_theta(wh);
	return std::exp(-tan2_t * (tangent_space::cos2_phi(wh) / (alpha * alpha) +
				   tangent_space::sin2_phi(wh) / (alpha * alpha))) /
	  (M_PI * alpha * alpha * cos4_t);
      }
    };
  }

  namespace shadowing {

    struct ggx_t {
      float_t a2;

      inline ggx_t(float_t alpha)
	: a2(alpha*alpha)
      {}

      inline float_t operator()(const float_t& cos_to) const {
	if (cos_to < 0) {
	  return 0;
	}
	
	const auto dnv = cos_to * 2;
	const auto nv2 = square(cos_to);

	return dnv / (cos_to * std::sqrt(a2+(1-a2)*nv2));
      }
    };

    struct ggx_pbrt_t {
      float_t a2;

      inline ggx_pbrt_t(float_t alpha)
	: a2(alpha*alpha)
      {}

      inline float_t operator()(const vector_t& w) const {
	const auto abs_tan_t = std::abs(tangent_space::tan_theta(w));
	if (std::isinf(abs_tan_t)) return 0.;
	// Compute _alpha_ for direction _w_
	const auto alpha =
	  std::sqrt(tangent_space::cos2_phi(w) * a2 + tangent_space::sin2_phi(w) * a2);
	
	const auto alpha2_tan2_t = (alpha * abs_tan_t) * (alpha * abs_tan_t);
	return (-1 + std::sqrt(1.f + alpha2_tan2_t)) / 2;
      }
    };

    /**
     * Schlick approximation to smith shadowing function
     *
     */
    struct schlick_t {
      float_t k;

      inline schlick_t(float_t alpha)
	: k(alpha * std::sqrt(M_2_PI))
      {}

      inline float_t operator()(const float_t& cos_to) const {
	return cos_to/cos_to*(1-k)+k;
      }
    };
  }
}
