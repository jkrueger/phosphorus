#pragma once

#include "precision.hpp"
#include "thing.hpp"
#include "math/orthogonal_base.hpp"
#include "util/color.hpp"

struct light_t {
  typedef std::shared_ptr<light_t> p;

  surface_t::p surface;
  float_t      area;
  color_t      emissive;
  vector_t     position;

  light_t(const vector_t& p, const surface_t::p& s, const color_t& e)
    : surface(s),
      area(0.0),
      emissive(e),
      position(p)
  {}

  void sample(
    const vector_t& p,
    const sample_t* samples,
    sampled_vector_t* out, uint32_t n) const {

    surface->sample(p, samples, out, n);

    orthogonal_base base((p - position).normalize());

    for (auto i=0; i<n; ++i) {
      out[i].sampled = base.to_world(out[i].sampled) + position;
    }
  }

  const color_t& emit() const {
    return emissive;
  }

  color_t power() const {
    return emissive * area * M_PI;
  }
};
