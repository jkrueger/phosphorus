#pragma once

#include "precision.hpp"
#include "util/algo.hpp"

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

  struct conductor_t {
    float_t etaI, etaT, k;

    inline conductor_t(float_t eI, float_t eT)
      : etaI(eI), etaT(eT)
    {}

    inline float_t operator()(float_t cos_ti) const {
      cos_ti = clamp(cos_ti, -1.0f, 1.0f);

      const auto eta    = etaT / etaI;
      const auto eta_k  = k / etaI;

      const auto cos_ti2 = cos_ti * cos_ti;
      const auto sin_ti2 = 1.0f - cos_ti2;
      const auto eta2    = eta * eta;
      const auto eta_k2  = eta_k * eta_k;

      const auto t0   = eta2 - eta_k2 - sin_ti2;
      const auto a2b2 = std::sqrt(t0*t0+4*eta2*eta_k2);
      const auto t1   = a2b2 + cos_ti2;
      const auto a    = std::sqrt(0.5f * (a2b2 + t0));
      const auto t2   = 2.0f * cos_ti * a;
      const auto rs   = (t1 - t2) / (t1 + t2);
      const auto t3   = cos_ti2 * a2b2 + sin_ti2 * sin_ti2;
      const auto t4   = t2 * sin_ti2;
      const auto rp   = rs * (t3 - t4) / (t3 + t4);

      return 0.5f * (rp + rs);
    }
  };

  // schlick approximation to fresnel equations
  struct schlick_t {
    inline color_t operator()(const color_t& r, float_t a) const {
      float cos_t = clamp(a, -1.0f, 1.0f);
      return r + (1-r)*(1-a);
    }
  };
}
