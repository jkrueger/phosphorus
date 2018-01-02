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
  double   u, v;
  double   pdf;
  vector_t p;

  sample_t()
    : u(rng::dis(rng::gen)), v(rng::dis(rng::gen))
  {}
};

static inline void cosine_sample_hemisphere(sample_t& sample) {
  const double r = std::sqrt(sample.u);
  const double theta = 2 * M_PI * sample.v;
 
  const double x = r * std::cos(theta);
  const double y = r * std::sin(theta);
 
  sample.p = vector_t(x, std::sqrt(std::max(0.0, 1.0 - sample.u)), y);
  sample.pdf = sample.p.y;
}
