#pragma once

#include "precision.hpp"
#include "math/vector.hpp"

#include <cmath>
#include <random> 
#include <functional> 

namespace rng {
  static thread_local std::random_device rd;
  static thread_local std::mt19937 gen(rd());
  static thread_local std::uniform_real_distribution<float_t> dis(0.0f,1.0f);
}

struct sample_t {
  float_t u, v;

  inline sample_t()
    : u(0.0), v(0.0)
  {}

  inline sample_t(float_t _u, float_t _v)
    : u(_u), v(_v)
  {}
};

template<typename T>
struct sampled_t {
  T      sampled;
  float_t pdf;
};

typedef sampled_t<vector_t> sampled_vector_t;

namespace sampling {
  namespace strategies {

    static const float_t UNIFORM_DISC_PDF = 1.0f / M_PI; 

    inline void uniform_sample_hemisphere(const sample_t& sample, sampled_vector_t& out) {
      const float r   = std::sqrt(1.0 - sample.u * sample.u);
      const float phi = 2 * M_PI * sample.v;

      out.sampled = vector_t(cos(phi) * r, sample.u, sin(phi) * r);
      out.pdf     = UNIFORM_DISC_PDF;
    }

    inline void cosine_sample_hemisphere(const sample_t& sample, sampled_vector_t& out) {
      const float_t r = std::sqrt(sample.u);
      const float_t theta = 2 * M_PI * sample.v;
 
      const float_t x = r * std::cos(theta);
      const float_t y = r * std::sin(theta);
 
      out.sampled = vector_t(x, std::sqrt(std::max(0.0f, 1.0f - sample.u)), y);
      out.pdf     = out.sampled.y * UNIFORM_DISC_PDF;
    }

    inline void stratified_2d(sample_t* samples, uint32_t num) {
      const float_t step = 1.0f / (float_t)num;
      float_t dy = 0.0;
      for (auto i=0; i<num; ++i, dy += step) {
	float_t dx = 0.0;
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
