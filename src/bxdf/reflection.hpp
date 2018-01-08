#pragma once

#include "shading.hpp"
#include "util/algo.hpp"

#include <algorithm>

namespace fresnel {
  struct none_t {
    inline color_t operator()(double a) const {
      return color_t(1, 1, 1);
    }
  };

  struct dielectric_t {
    double etaI, etaT;

    inline dielectric_t(double eI, double eT)
      : etaI(eI), etaT(eT)
    {}

    inline color_t operator()(double a) const {
      auto eI = etaI, eT = etaT; 
      auto cosTI    = clamp(a, -1.0, 1.0);
      bool entering = cosTI > 0;
      if (entering) {
	std::swap(eI, eT);
      }
      auto sinI = std::sqrt(std::max(0.0, 1.0 - cosTI * cosTI));
      auto sinT = eI / eT * sinI;
      if (sinT >= 1) {
	return 1;
      }
      auto cosTT = std::sqrt(std::max(0.0, 1.0 - sinT * sinT));

      auto r_parl =
	((eT * cosTI) - (eI * cosTT)) /
	((eT * cosTI) + (eI * cosTT));
      auto r_perp =
	((eI * cosTI) - (eT * cosTT)) /
	((eI * cosTI) + (eT * cosTT));

      return (r_parl * r_parl + r_perp * r_perp) * 0.5;
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

    double pdf(const vector_t&, const vector_t&) const {
      return 0.0;
    }
  };

  template<typename Fresnel>
  struct specular_transmission_t : public bxdf_t {
    Fresnel fresnel;
    double  etaA, etaB;
    color_t k;

    template<typename... Args>
    specular_transmission_t(double eA, double eB, const Args& ...args)
      : bxdf_t(bxdf_t::TRANSMISSIVE), fresnel(eA, eB, args...)
    {}

    color_t sample(const vector_t& v, const sample_t& sample, sampled_vector_t& out) const {
      auto cos_n = v.y;
      bool entering = cos_n > 0;
      auto etaI = entering ? etaA : etaB;
      auto etaT = entering ? etaB : etaA;
      auto eta  = etaI / etaT;

      out.pdf = 1;

      auto cosTI  = v.y;
      auto sin2TI = std::max(0.0, 1.0 - cosTI * cosTI);
      auto sin2TT = eta * eta * sin2TI;

      if (sin2TT >= 1) {
	return 0;
      }

      auto cosTT = std::sqrt(1 - sin2TT);

      out.sampled = eta * -v + (eta * cosTI - cosTT) * vector_t(0, v.y > 0.0 ? 1 : -1, 0);

      // TODO: account for transmission from denser medium to less dense
      // (e.g. from water to air)

      auto spectrum = k * (color_t(1.0) - fresnel(out.sampled.y));

      return spectrum * (1.0 / std::abs(out.sampled.y));
    }

    double pdf(const vector_t&, const vector_t&) const {
      return 0.0;
    }
  };
}
