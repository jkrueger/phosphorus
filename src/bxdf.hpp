#pragma once

#include "math/sampling.hpp"
#include "math/vector.hpp"
#include "util/color.hpp"

#include <stdexcept>

struct bxdf_t {
  typedef bxdf_t* p;

  enum flags_t {
    NONE         = 0,
    DIFFUSE      = (1 << 0),
    SPECULAR     = (1 << 1),
    REFLECTIVE   = (1 << 2),
    TRANSMISSIVE = (1 << 3)
  };

  uint32_t flags;

  inline bxdf_t(flags_t f)
    : flags(f)
  {}

  virtual color_t f(const vector_t& in, const vector_t& out) const {
    return color_t();
  }

  virtual color_t sample(const vector_t& v, const sample_t& sample, sampled_vector_t& out) const {
    throw std::runtime_error("Sample not implemented for this bxdf");
  }

  virtual float_t pdf(const vector_t& in, const vector_t& out) const {
    return 0.0;
  }

  inline bool is(flags_t mode) const {
    return (flags & mode) != 0;
  }

  inline bool is_diffuse() const {
    return is(DIFFUSE);
  }

  inline bool is_specular() const {
    return is(SPECULAR);
  }

  /**
   * The bxdf has a component that reflects light potentially in a non 
   * uniform way over the hemisphere
   *
   */
  inline bool has_distribution() const {
    return is_diffuse() || is_specular();
  }

  inline bool is_reflective() const {
    return is(REFLECTIVE);
  }

  inline bool is_transmissive() const {
    return is(TRANSMISSIVE);
  }
};
