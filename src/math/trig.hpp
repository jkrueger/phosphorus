#pragma

#include "precision.hpp"
#include "vector.hpp"
#include "util/algo.hpp"

#include <algorithm>
#include <cmath>

/**
 * Optimized trigonometry functions in tagent space (the normal is (0,1,0))
 *
 */
namespace tangent_space {
  inline float_t cos2_theta(const vector_t& v) {
    return v.y*v.y;
  }

  inline float_t cos_theta(const vector_t& v) {
    return v.y;
  }

  inline float_t sin2_theta(const vector_t& v) {
    return std::max((float_t) 0, (float_t)1 - cos2_theta(v));
  }

  inline float_t sin_theta(const vector_t& v) {
    return std::sqrt(sin2_theta(v));
  }

  inline float_t tan_theta(const vector_t& v) {
    return sin_theta(v) / cos_theta(v);
  }

  inline float_t tan2_theta(const vector_t& v) {
    return sin2_theta(v) / cos2_theta(v);
  }

  inline float_t cos_phi(const vector_t& v) {
    auto sin_t = sin_theta(v);
    return (sin_t == 0) ? 1 : clamp(v.z / sin_t, -1.f, 1.f);  
  }

  inline float_t cos2_phi(const vector_t& v) {
    return square(cos_phi(v));
  }

  inline float_t sin_phi(const vector_t& v) {
    float_t sin_t = sin_theta(v);
    return (sin_t == 0) ? 0 : clamp(v.x / sin_t, -1.f, 1.f);
  }

  inline float_t sin2_phi(const vector_t& v) {
    return square(sin_phi(v));
  }
}
