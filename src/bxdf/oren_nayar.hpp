#pragma once

#include "bxdf.hpp"
#include "math/trig.hpp"

namespace bxdf {
  struct oren_nayar_t : public bxdf_t {
    color_t r;
    float_t a, b;

    oren_nayar_t(color_t r, float_t s)
      : bxdf_t(flags_t::DIFFUSE), r(r) {
      auto sigma  = s * (M_PI / 180.0f);
      auto sigma2 = sigma * sigma;
      a = 1.0f - (sigma2 / (2.0f * (sigma2 + 0.33f)));
      b = 0.45f * sigma2 / (sigma2 + 0.09f);
    }

    color_t f(const vector_t& in, const vector_t& out) const {
      using namespace tangent_space;
      
      float_t sin_theta_i = sin_theta(in);
      float_t sin_theta_o = sin_theta(out);

      float_t max_cos = 0;
      if (sin_theta_i > 0.0001f && sin_theta_o > 0.0001f) {
	float_t sin_phi_i = sin_phi(in),  cos_phi_i = cos_phi(in);
	float_t sin_phi_o = sin_phi(out), cos_phi_o = cos_phi(out);

	float_t dcos = cos_phi_i * cos_phi_o + sin_phi_i * sin_phi_o;

	max_cos = std::max(0.0f, dcos);
      }

      float_t sin_alpha, tan_beta;
      if (std::abs(in.y) > std::abs(out.y)) {
	sin_alpha = sin_theta_o;
	tan_beta  = sin_theta_i / std::abs(in.y);
      }
      else {
	sin_alpha = sin_theta_i;
	tan_beta  = sin_theta_o / std::abs(out.y);
      }

      return r * (1.0f/M_PI) * (a + b * max_cos * sin_alpha * tan_beta);
    }

    color_t sample(const vector_t& v, const sample_t& sample, sampled_vector_t& out) const {
      sampling::hemisphere::cosine_weighted(sample, out);
      return f(out.sampled, v);
    }

    float_t pdf(const vector_t& in, const vector_t& out) const {
      return in.y * (1.0 / M_PI);
    }
  };
}
