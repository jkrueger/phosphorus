#pragma once

#include "precision.hpp"

#include <algorithm>

namespace fresnel {
  struct none_t {
    inline float_t operator()(float_t a) const {
      return 1.0f;
    }
  };

  struct dielectric_t {
    float_t etaI, etaT;

    inline dielectric_t(float_t eI, float_t eT)
      : etaI(eI), etaT(eT)
    {}

    inline float_t operator()(float_t a) const {
      auto eI    = etaI, eT = etaT; 
      auto cosTI = clamp(a, -1.0f, 1.0f);
      if (cosTI <= 0) {
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

  // schlick approximation to fresnel equation
}
