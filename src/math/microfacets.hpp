#pragma once

#include "precision.hpp"
#include "trig.hpp"
#include "vector.hpp"

namespace microfacet {
  namespace distribution {
    /**
     * Trowbridge-Reitz microfacet distribution
     *
     */
    struct ggx_t {
      float_t alpha;

      inline float_t operator()(const vector_t& m) const {
	const auto a2  = square(alpha);
	const auto nm2 = square(tangent_space::cos_theta(m)));

	return 1.0f/(M_PI*(nm2*(a2-1)+1));
      }

      // inline vector_t sample() const {}
    };
  }

  namespace shadowing {
    /**
     * Schlick approximation to smith shadowing function
     *
     */
    struct schlick_t {
      static const float_t k0 = std::sqrt(M_2_PI);
      
      float_t k;

      inline schlick_t(float_t alpha)
	: k(alpha * k0)
      {}

      inline float_t operator()(const float_t& cos_to) const {
	return cos_to/cos_to*(1-k)+k;
      }
    };
  }
}
