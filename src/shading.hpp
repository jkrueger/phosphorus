#pragma once

#include "math/ray.hpp"
#include "math/vector.hpp"
#include "material.hpp"
#include "thing.hpp"
#include "util/color.hpp"

#include <limits>

struct bxdf_t {
  typedef std::shared_ptr<bxdf_t> p;
  
  virtual color_t f(const vector_t& in, const vector_t& out, const vector_t& n) const = 0;
  // TODO: sample
};

struct shading_info_t {
  double   d;
  vector_t p;
  vector_t n;
  
  const shadable_t* thing;

  inline shading_info_t()
    : d(std::numeric_limits<double>::max())
  {}

  template<typename T>
  inline bool update(const ray_t& ray, double _d, const T& shadable) {
    if (_d < d) {
      d = _d;
      p = ray.at(d);
      thing = static_cast<const shadable_t*>(&shadable);
      shadable.shading_parameters(*this, p);
      return true;
    }
    return false;
  }

  bxdf_t::p bxdf() const {
    return thing->material->at(*this);
  }
};
