#pragma once

#include "precision.hpp"
#include "bxdf.hpp"
#include "math/fresnel.hpp"
#include "util/algo.hpp"

#include <algorithm>

namespace bxdf {
  struct specular_reflection_t : public bxdf_t {
    color_t k;

    specular_reflection_t(const color_t& k)
      : k(k), bxdf_t(bxdf_t::REFLECTIVE)
    {}

    color_t sample(const vector_t& v, const sample_t& sample, sampled_vector_t& out) const {
      out.sampled = vector_t(-v.x, v.y, -v.z);
      out.pdf     = 1.0;
      auto cos_n = v.y;

      return k * (1.0f / std::abs(cos_n));
    }

    float_t pdf(const vector_t&, const vector_t&) const {
      return 0.0;
    }
  };

  struct specular_transmission_t : public bxdf_t {
    float_t etaA, etaB;
    color_t k;

    specular_transmission_t(const color_t& k, float_t eA, float_t eB)
      : bxdf_t(bxdf_t::TRANSMISSIVE), k(k), etaA(eA), etaB(eB)
    {}

    color_t sample(const vector_t& v, const sample_t& sample, sampled_vector_t& out) const {
      const auto cos_n = v.y;
      bool entering = cos_n > 0;
      const auto etaI = entering ? etaA : etaB;
      const auto etaT = entering ? etaB : etaA;
      const auto eta  = etaI / etaT;

      out.pdf = 1;

      const auto cosTI  = v.y;
      const auto sin2TI = std::max(0.0, 1.0 - cosTI * cosTI);
      const auto sin2TT = eta * eta * sin2TI;

      if (sin2TT >= 1) {
	return 0;
      }

      const auto cosTT = std::sqrt(1 - sin2TT);

      const vector_t n(0, entering ? 1 : -1, 0);
      out.sampled = eta * -v + (eta * cosTI - cosTT) * n;

      return k * (1.0 / std::abs(out.sampled.y));
    }

    float_t pdf(const vector_t&, const vector_t&) const {
      return 0.0;
    }
  };
}
