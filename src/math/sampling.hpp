#pragma once

#include "math/vector.hpp"

#include <cmath>
#include <random> 
#include <functional> 

namespace rng {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<> dis(0.0,1.0);
}

struct sample_t {
  double u, v;

  inline sample_t()
    : u(0.0), v(0.0)
  {}

  inline sample_t(double _u, double _v)
    : u(_u), v(_v)
  {}
};

template<typename T>
struct sampled_t {
  T      sampled;
  double pdf;
};

typedef sampled_t<vector_t> sampled_vector_t;

namespace sampling {
  namespace strategies {

    static const double UNIFORM_DISC_PDF = 1.0 / M_PI; 

    inline void uniform_sample_hemisphere(const sample_t& sample, sampled_vector_t& out) {
      const float r   = std::sqrt(1.0 - sample.u * sample.u);
      const float phi = 2 * M_PI * sample.v;

      out.sampled = vector_t(cos(phi) * r, sample.u, sin(phi) * r);
      out.pdf     = UNIFORM_DISC_PDF;
    }

    inline void cosine_sample_hemisphere(const sample_t& sample, sampled_vector_t& out) {
      const double r = std::sqrt(sample.u);
      const double theta = 2 * M_PI * sample.v;
 
      const double x = r * std::cos(theta);
      const double y = r * std::sin(theta);
 
      out.sampled = vector_t(x, std::sqrt(std::max(0.0, 1.0 - sample.u)), y);
      out.pdf     = out.sampled.y * UNIFORM_DISC_PDF;
    }

    inline void stratified_2d(sample_t* samples, uint32_t num) {
      const double step = 1.0 / (double)num;
      double dy = 0.0;
      for (auto i=0; i<num; ++i, dy += step) {
	double dx = 0.0;
	for (auto j=0; j<num; ++j, dx += step) {
	  samples[j * num + i] = {
	    dx + rng::dis(rng::gen) * step,
	    dy + rng::dis(rng::gen) * step
	  };
	}
      }
    }
  }
}
