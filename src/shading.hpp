#pragma once

#include "precision.hpp"
#include "math/orthogonal_base.hpp"
#include "math/ray.hpp"
#include "math/vector.hpp"
#include "material.hpp"
#include "thing.hpp"
#include "util/color.hpp"

#include <limits>

struct bxdf_t {
  typedef std::shared_ptr<bxdf_t> p;

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

struct shading_info_t {
  float_t          d;
  vector_t        p;
  vector_t        n;
  invertible_base b;
  bxdf_t::p       _bxdf;
  color_t         emissive;

  const shadable_t* thing;

  inline shading_info_t()
    : d(std::numeric_limits<float_t>::max())
  {}

  template<typename T, typename... XS>
  inline bool update(const ray_t& ray, float_t _d, const T* shadable, const XS& ...xs) {
    if (_d < d) {
      d = _d;
      p = ray.at(d);
      thing = static_cast<const shadable_t*>(shadable);
      shadable->shading_parameters(*this, p, xs...);
      b  = invertible_base(n);
      return true;
    }
    return false;
  }

  inline bxdf_t::p& bxdf() {
    if (!_bxdf) {
      _bxdf = thing->material->at(*this);
    }
    return _bxdf;
  }
};
