#pragma once

#include "math/vector.hpp"

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
