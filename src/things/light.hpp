#pragma once

#include "thing.hpp"

struct light_t : public thing_t {

  typedef std::shared_ptr<light_t> p;

  thing_t::p thing;
  double     area;
  color_t    emit;

  light_t(thing_t::p t, const color_t& e)
    : thing_t(t->position), thing(t), area(0.0), emit(e)
  {}

  bool intersect(const ray_t& ray, shading_info_t& info) const {
    return thing->intersect(ray, info);
  }

  void sample(const vector_t& p, sample_t* out, uint32_t n) const {
    thing->sample(p, out, n);
  }

  color_t power() const {
    return emit * area * M_PI;
  }
};
