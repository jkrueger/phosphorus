#pragma once

#include "precision.hpp"
#include "shading.hpp"
#include "util/algo.hpp"

#include <algorithm>

namespace fresnel {
  struct none_t {
    inline color_t operator()(float_t a) const {
      return color_t(1.f, 1.f, 1.f);
    }
  };

  struct dielectric_t {
    float_t etaI, etaT;

    inline dielectric_t(float_t eI, float_t eT)
      : etaI(eI), etaT(eT)
    {}

    inline color_t operator()(float_t a) const {
      auto eI = etaI, eT = etaT; 
      auto cosTI    = clamp(a, -1.0f, 1.0f);
      bool entering = cosTI > 0;
      if (!entering) {
	std::swap(eI, eT);
	cosTI = std::abs(cosTI);
      }
      auto sinI = std::sqrt(std::max(0.0f, 1.0f - cosTI * cosTI));
      auto sinT = eI / eT * sinI;
      if (sinT >= 1) {
	return 1;
      }
      auto cosTT = std::sqrt(std::max(0.0f, 1.0f - sinT * sinT));

      auto r_parl =
	((eT * cosTI) - (eI * cosTT)) /
	((eT * cosTI) + (eI * cosTT));
      auto r_perp =
	((eI * cosTI) - (eT * cosTT)) /
	((eI * cosTI) + (eT * cosTT));

      return (r_parl * r_parl + r_perp * r_perp) * 0.5f;
    }
  };
}

namespace bxdf {
  template<typename Fresnel>
  struct specular_reflection_t : public bxdf_t {
    Fresnel fresnel;
    color_t k;

    template<typename... Args>
    specular_reflection_t(const Args& ...args)
      : bxdf_t(bxdf_t::REFLECTIVE), fresnel(args...)
    {}

    color_t sample(const vector_t& v, const sample_t& sample, sampled_vector_t& out) const {
      out.sampled = vector_t(-v.x, v.y, -v.z);
      out.pdf     = 1.0;
      auto cos_n = v.y;
      return (k * fresnel(cos_n)) * (1.0 / std::abs(cos_n));
    }

    float_t pdf(const vector_t&, const vector_t&) const {
      return 0.0;
    }
  };

  template<typename Fresnel>
  struct specular_transmission_t : public bxdf_t {
    Fresnel fresnel;
    float_t etaA, etaB;
    color_t k;

    template<typename... Args>
    specular_transmission_t(float_t eA, float_t eB, const Args& ...args)
      : bxdf_t(bxdf_t::TRANSMISSIVE), fresnel(eA, eB, args...)
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

      const auto spectrum = k * (color_t(1.0) - fresnel(out.sampled.y));

      return spectrum * (1.0 / std::abs(out.sampled.y));
    }

    float_t pdf(const vector_t&, const vector_t&) const {
      return 0.0;
    }
  };
}
