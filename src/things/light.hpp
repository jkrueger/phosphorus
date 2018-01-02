#pragma once

#include "thing.hpp"

struct light_t : public thing_t {
  typedef std::shared_ptr<light_t> p;

  shadable_t::p thing;
  double        area;

  light_t(shadable_t::p t, const color_t& e)
    : thing_t(t->position), thing(t), area(0.0)
  {
    thing->emissive = e;
  }

  bool intersect(const ray_t& ray, shading_info_t& info) const {
    return thing->intersect(ray, info);
  }

  void sample(const vector_t& p, sample_t* out, uint32_t n) const {
    thing->sample(p, out, n);
  }

  color_t& emit() const {
    return thing->emissive;
  }

  color_t power() const {
    return thing->emissive * area * M_PI;
  }
};
