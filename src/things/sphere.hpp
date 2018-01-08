#pragma once

#include "thing.hpp"

#include "math/sampling.hpp"
#include "math/vector.hpp"

struct sphere_t : public shadable_t {
  double   radius;
  double   radius2;

  sphere_t(const vector_t& c, double r, const material_t::p& m);

  bool intersect(const ray_t& ray, shading_info_t& info) const;

  void shading_parameters(shading_info_t& info, const vector_t& p) const;

  void sample(
    const vector_t& p,
    const sample_t* samples,
    sampled_vector_t* sampled,
    uint32_t num) const;
};
