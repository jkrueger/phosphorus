#pragma once

#include "thing.hpp"

struct light_t : public thing_t {
  typedef std::shared_ptr<light_t> p;

  shadable_t::p thing;
  double        area;
  color_t       emissive;

  light_t(shadable_t::p t, const color_t& e)
    : thing_t(t->position),
      thing(t),
      area(0.0),
      emissive(e)
  {}

  bool intersect(const ray_t& ray, shading_info_t& info) const {
    if (thing->intersect(ray, info)) {
      info.emissive = emissive;
      return true;
    }
    return false;
  }

  void sample(const vector_t& p, const sample_t* samples, sampled_vector_t* out, uint32_t n) const {
    thing->sample(p, samples, out, n);
  }

  const color_t& emit() const {
    return emissive;
  }

  color_t power() const {
    return emissive * area * M_PI;
  }
};
