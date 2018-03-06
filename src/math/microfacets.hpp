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
	roughness = std::max(roughness, (float_t)1e-3);
	float_t x = std::log(roughness);
	return 1.62142f
	  + 0.819955f * x
	  + 0.1734f * x * x
	  + 0.0171201f * x * x * x
	  + 0.000640711f * x * x * x * x;
      }
    };
  }

  namespace shadowing {
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
